#include "sched.h"
#include <stdlib.h>
#include "sched.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h> /* For va_start() */
#include <limits.h>
#include "barrier.h"

#define MAX_CHARS_TASK_LINE 50

typedef enum {NEW_TASK,TASK_AWAKEN} sched_event_type_t;

typedef struct {
    task_t* task;
    sched_event_type_t type;
    int timeout;     		/* The simulation step when the event actually fires*/
    list_node_t links;		/* Node for the linked list */
} sched_event_t;


runqueue_t runqueues[MAX_CPUS];	/* Per-CPU run queues  */
slist_t sched_events[MAX_CPUS]; /* This is where waiting tasks and newly created ones
				 * are stored until they are eligible to run
				 *
				 * */
FILE* cpu_log[MAX_CPUS];	/* Per-CPU log files */


/* Critical resources for the simulation */
sys_barrier_t simulation_barrier;
pthread_mutex_t simulation_lock;
sched_class_t* active_sched_class=&rr_sched; /* Default RR */
int cpus_completed=0;

/* Global scheduling parameters */
unsigned int nr_cpus=1;
bool debug_mode=FALSE;
useconds_t tick_delay=250000;
int max_simulation_steps=50;
bool preemptive_scheduler=FALSE;
int load_balancing_period=5; /* Default */

/* Main scheduler functions */
static void scheduler_tick(int cpu,int simulation_step);
static void schedule(int cpu,int simulation_step);
static int process_sched_events(int cpu,int simulation_step);
static void put_task_to_sleep(task_t* task, int timeout);
static void  schedule_wake_up_new_task(task_t* task);

/* Functions to handle basic run queue locking */
static inline void lock_rq(runqueue_t* rq)
{
    pthread_mutex_lock(&rq->lock);
}
static inline void unlock_rq(runqueue_t* rq)
{
    pthread_mutex_unlock(&rq->lock);
}

/* Adquire the lock of a remote runqueue safely
    (We assume the lock of this_rq is already held)
   This function is used by the load balancer
*/
static inline void double_lock_rq(runqueue_t* this_rq,runqueue_t* remote_rq)
{

    /* If it's a smaller CPU ID, simply adquire the lock
     * Otherwise, unlock this_rq first and lock both RQs following
     * the locking protocol
     */
    if (this_rq > remote_rq) {
        lock_rq(remote_rq);
    } else {
        unlock_rq(this_rq);
        lock_rq(remote_rq);
        lock_rq(this_rq);
    }
}


/* This function parses an input file describing the various tasks.
 * On success, it returns a task list (task_t).
 *
 */
slist_t read_task_list_from_file(FILE *file)
{
    slist_t slist;
    char line[MAX_CHARS_TASK_LINE]="";
    char cpbuf[MAX_CHARS_TASK_LINE]="";
    char* parsed_line;
    char* token=NULL;
    int idx_token=0;
    task_t* cur_task=NULL;
    exec_phase_t* phase=NULL;
    int cnt_tasks=0;
    int line_counter=0;
    int error=0;

    init_slist (&slist, offsetof(task_t,ts_links));

    /*- Formato del cada linea:
        NombreTarea Prio startTime <CPU1> <IO1> <CPU2> <IO2> ...
    */
    while(!feof(file) && fgets(line, MAX_CHARS_TASK_LINE, file)) {
        line_counter++;

        if (strcmp(line, "\n")) {
            /* Copy line in temporary storage */
            parsed_line=cpbuf;
            strcpy(parsed_line,line);

            /* Allocate memory for task descriptor */
            cur_task=(task_t*)malloc(sizeof(task_t));
            memset(cur_task,0,sizeof(task_t));
            cur_task->task_id=cnt_tasks++;

            idx_token=0;

            while(!error && (token= strsep(&parsed_line," "))!=NULL) {
                switch(idx_token) {
                case 0:
                    strcpy(cur_task->task_name,token);
                    break;
                case 1:
                    if (sscanf(token,"%d",&cur_task->prio)!=1)
                        error=1;
                    break;
                case 2:
                    if (sscanf(token,"%d",&cur_task->task_profile.arrival_time)!=1)
                        error=1;
                    break;
                default:
                    phase=&cur_task->task_profile.phase[cur_task->task_profile.nr_phases];
                    if (cur_task->task_profile.nr_phases & 0x1)  //odd == IO
                        phase->type=IO_PHASE;
                    else
                        phase->type=CPU_PHASE;

                    /* Ignore blank spaces */
                    if (sscanf(token,"%d",&phase->len)==1) {
                        //Add phase
                        cur_task->task_profile.nr_phases++;
                    }

                    break;
                }
                idx_token++;
            }

            if (error) {
                fprintf(stderr,"Syntax error in input file (line %d) -> %s \n",line_counter,line);
                exit(1);
            }

            if (cur_task->task_profile.nr_phases==0) {
                fprintf(stderr,"Error parsing CPU/IO pattern for task (line %d) -> %s \n",line_counter,line);
                exit(1);
            }

            //Add task to the list
            insert_slist(&slist,cur_task);
        }
    }
    return slist;
}

/* Add a log register for the post simulation analysis */
static void add_log_register(task_t* task, task_state_t state, int cpu, int when, int how_long)
{
    sched_log_t* log_reg=tail_slist(&task->sched_regs);
    sched_log_t* new_reg;

    /* Do not insert empty registers */
    if (how_long==0)
        return;

    if (!log_reg || log_reg->state!=state || log_reg->cpu!=cpu ) {
        new_reg=(sched_log_t*)malloc(sizeof(sched_log_t));
        new_reg->state=state;
        new_reg->cpu=cpu;
        new_reg->when=when;
        new_reg->how_long=how_long;

        insert_slist(&task->sched_regs,new_reg);
    } else {
        /* If thread was runnable and now it's ONPROC -> Update the previous register */
        log_reg->how_long+=how_long;
    }
}

//TS_STARTING=0,TS_RUNNABLE,TS_ONPROC,TS_SLEEP,TS_COMPLETED
static const char* strstate[]= {"start","ready","running","I/O","completed"};

static void print_task_log_registers(task_t* task)
{
    sched_log_t* cur=head_slist(&task->sched_regs);
    sched_log_t* next;

    /* Disable debug mode for this */
    debug_mode=FALSE;

    while(!is_empty_slist(&task->sched_regs)) {
        write_cpu_log(cur->cpu,"%s\t%d\t%d\t%s\n",task->task_name,cur->when,cur->when+cur->how_long,strstate[cur->state]);
        /* Keep track of next, since we will remove cur from the list */
        next=next_slist(&task->sched_regs,cur);
        remove_slist(&task->sched_regs,cur);
        free(cur);
        cur=next;// Go to next item
    }
}

/* Initialize scheduler-specific fields for the task */
static int init_task_sched(task_t* task)
{

    init_slist (&task->sched_regs, offsetof(sched_log_t,links));

    task->tcs_data=NULL; /* Initialize class-specific data (default: no data) */

    if (!(task->flags & TF_IDLE_TASK) && active_sched_class->task_new)
        return active_sched_class->task_new(task); //class-specific initialization
    else
        return 0;
}

/* Initialize a per-CPU run queue */
static void init_runqueue(runqueue_t* rq, int cpu)
{
    init_slist(&rq->tasks, offsetof(task_t,rq_links));
    /* Initialize the init task */
    memset(&rq->idle_task,0,sizeof(task_t));
    rq->idle_task.flags|=TF_IDLE_TASK;	/* Enable the IDLE FLAG */
    rq->idle_task.state=TS_RUNNABLE; 	/* The idle task is always runnable */
    strcpy(rq->idle_task.task_name,"idle");
    init_task_sched(&rq->idle_task);
    rq->cur_task=&rq->idle_task;
    rq->need_resched=FALSE;
    rq->nr_runnable=0;
    rq->next_load_balancing=cpu%nr_cpus;
    rq->rq_cs_data=NULL; /* Initialize class-specific data (default: no data) */
    pthread_mutex_init(&rq->lock,NULL);
}

void sched_terminate(int errcode)
{
    /* Close */
    int cpu=0;

    /* Close file descriptors */
    fprintf(stderr,"Closing file descriptors...\n");

    while(cpu<nr_cpus && cpu_log[cpu]!=NULL) {
        fclose(cpu_log[cpu]);
        cpu++;
    }

    if (!errcode) {
        sys_barrier_destroy(&simulation_barrier);
        pthread_mutex_destroy(&simulation_lock);
    }
    exit(errcode);
}

/* This function makes it possible to print a
 * debug/log message in a specific per-CPU log file
 *
 * */
int write_cpu_log(int cpu, const char *fmt, ...)
{
    va_list args;
    int i;

    if (debug_mode) {
        printf("CPU %d:",cpu);
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
    }

    va_start(args, fmt);
    i = vfprintf(cpu_log[cpu] , fmt, args);
    va_end(args);
    return i;
}

/*
 * Initialize global scheduling structures
 */
static void sched_init(struct sched_class* sc)
{
    int cpu=0;
    char logfile_name[10]="";

    for (cpu=0; cpu<nr_cpus; cpu++) {
        init_runqueue(&runqueues[cpu],cpu);
        init_slist(&sched_events[cpu],offsetof(sched_event_t,links));
    }

    active_sched_class=sc;

    /* Perform class-specific global initialization if required */
    if (active_sched_class->sched_init && active_sched_class->sched_init()) {
        perror("Couldn't initialize scheduling class");
        exit(1);
    }

    if( pthread_mutex_init(&simulation_lock,NULL) < 0) {
        perror("Can't create mutex");
        exit(1);
    }

    if( sys_barrier_init(&simulation_barrier,nr_cpus) < 0) {
        fprintf(stderr,"Can't create barrier");
        exit(1);
    }

    /* Open log files */
    /* NULL initialization */
    memset(cpu_log,0,MAX_CPUS*sizeof(FILE*));

    /* Delete existing files*/
    system("rm -f CPU_*.log");

    for (cpu=0; cpu<nr_cpus; cpu++) {

        sprintf(logfile_name,"CPU_%i.log",cpu);

        if((cpu_log[cpu] = fopen(logfile_name, "w")) == NULL) {
            fprintf(stderr,"Can't open file \n");
            sched_terminate(1);
        }
        setbuf(cpu_log[cpu],NULL);
    }

    setbuf(stdout,NULL);

}

static void print_task_statistics(task_t* t)
{
    printf("Statistics: task_name=%s real_time=%d user_time=%d io_time=%d\n",t->task_name,t->real_time,t->user_time,t->sys_time);
}

static void print_task(task_t* t)
{
    int i=0;

    printf("==== TASK %s ===\n", t->task_name);
    printf("Priority: %d\n",t->prio);
    printf("Arrival time: %d\n",t->task_profile.arrival_time);
    printf("Profile: [ ");
    for (i=0; i<t->task_profile.nr_phases; i++)
        printf("%d ",t->task_profile.phase[i].len);
    printf("]\n");
    printf("=================\n");
}


static void sched_task_completed(task_t* task)
{
    task->state=TS_COMPLETED;
    print_task_statistics(task);
}


sched_event_t* get_next_sched_event(int cpu, int timeout)
{

    sched_event_t* event=head_slist(&sched_events[cpu]);

    if (event!=NULL && timeout==event->timeout) {
        remove_slist(&sched_events[cpu],event);
        return event;
    } else
        return NULL;
}

/*
 * This function takes care of waking up tasks that become runnable
 * in the specified simulation_step on a certain CPU.
 * It returns the number of events processed (woken up tasks).
 */
int process_sched_events(int cpu, int simulation_step)
{
    int event_count=0;
    sched_event_t *event;
    runqueue_t* rq=get_runqueue_cpu(cpu);

    while((event=get_next_sched_event(cpu,simulation_step))) {
        task_t* task=event->task;
        exec_profile_t* task_profile=&task->task_profile;
        exec_phase_t* exec_phase=NULL;


        switch(event->type) {
        case NEW_TASK:
            exec_phase=&task_profile->phase[task_profile->cur_phase];
            if (exec_phase->type==IO_PHASE) {
                fprintf(stderr,"Critical error: IO Phase detected when awaking task of a task's profile\n");
                sched_terminate(1);
            }

            task->runnable_ticks_left=exec_phase->len;
            task->last_time_enqueued=simulation_step;
            task->state=TS_RUNNABLE;

            lock_rq(rq);
            active_sched_class->enqueue_task(event->task,cpu,0);
            unlock_rq(rq);

            write_cpu_log(cpu,"(t%d): New task %s\n",simulation_step,task->task_name);
            break;
        case TASK_AWAKEN:
            task_profile->cur_phase++;

            if (task_profile->cur_phase>=task_profile->nr_phases) {
                sched_task_completed(task);
            } else {

                exec_phase=&task_profile->phase[task_profile->cur_phase];

                if (exec_phase->type==IO_PHASE) {
                    fprintf(stderr,"Critical error: IO Phase detected when awaking task of a task's profile\n");
                    sched_terminate(1);
                }

                task->runnable_ticks_left=exec_phase->len;
                task->last_time_enqueued=simulation_step;
                task->state=TS_RUNNABLE;

                lock_rq(rq);
                active_sched_class->enqueue_task(event->task,cpu,0);
                unlock_rq(rq);

                write_cpu_log(cpu,"(t%d): task %s awoken\n",simulation_step,task->task_name);
            }
            break;
        }

        free(event);
        event_count++;
    }

    return event_count;
}

/* This is main tick-processing routine */
static void scheduler_tick(int cpu,int simulation_step)
{
    task_t* current;
    runqueue_t* cpu_rq=get_runqueue_cpu(cpu);

    /* Lock the Rq*/
    lock_rq(cpu_rq);
    current=cpu_rq->cur_task;

    /* Update global statistics */
    current->user_time++;
    current->real_time++;
    current->last_cpu=cpu;

    active_sched_class->task_tick(cpu_rq,cpu);

    add_log_register(current,TS_ONPROC,cpu,simulation_step,1);
    write_cpu_log(cpu,"(t%d): %s running\n",simulation_step,current->task_name);

    /* The task is either exiting or going to sleep right now */
    if (!is_idle_task(current) && (--current->runnable_ticks_left)==0) {

        cpu_rq->need_resched=TRUE;	/* Make sure the scheduler gets invoked !! */
        exec_profile_t* task_profile=&current->task_profile;
        exec_phase_t* exec_phase=NULL;
        task_profile->cur_phase++;

        if (task_profile->cur_phase>=task_profile->nr_phases) {
            sched_task_completed(current);
        } else {

            exec_phase=&task_profile->phase[task_profile->cur_phase];

            if (exec_phase->type==CPU_PHASE) {
                fprintf(stderr,"Critical error: CPU Phase detected when leaving the CPU\n");
                pthread_mutex_unlock(&cpu_rq->lock);
                sched_terminate(1);
            }

            /* Charge system and real time in advance*/
            current->sys_time+=exec_phase->len;
            current->real_time+=exec_phase->len;
            /* Change state */
            current->state=TS_SLEEP;
            /* Actually put this task to sleep and schedules when it wakes up */
            put_task_to_sleep(current,simulation_step+exec_phase->len+1);
            add_log_register(current,TS_SLEEP,cpu,simulation_step+1,exec_phase->len);
            write_cpu_log(cpu,"(t%d): Task %s goes to sleep until (t%d)\n",simulation_step+1,current->task_name,simulation_step+exec_phase->len+1);
        }
    }

    /* Unlock the Rq*/
    unlock_rq(cpu_rq);
}

/* This function performs the work associated with moving a task from one CPU to another.
 * The locks of both CPU run queues must be already held.
*/
static void move_one_task(runqueue_t* src_rq,int src_cpu,runqueue_t* dst_rq,int dst_cpu, int this_cpu, int simulation_step)
{
    task_t* stolen_task=NULL;
    int time_on_rq=0;

    /* Perform actual task migration */
    stolen_task=active_sched_class->steal_task(src_rq,src_cpu);
    active_sched_class->enqueue_task(stolen_task,dst_cpu,0);

    /* Keep track of how long the task was runnable
     * Update real time and add log register
    */
    time_on_rq=(simulation_step-stolen_task->last_time_enqueued+1);
    stolen_task->real_time+=time_on_rq;
    add_log_register(stolen_task,TS_RUNNABLE,src_cpu,stolen_task->last_time_enqueued,time_on_rq);
    write_cpu_log(this_cpu,"(t%d): Task %s migrated from CPU %d to CPU %d \n", simulation_step,stolen_task->task_name,src_cpu,dst_cpu);
    stolen_task->last_time_enqueued=simulation_step+1; /* It will be enqueued starting the next interval */
}

/* Performs load balancing from this_cpu
 * if it's due time or the CPU is idle
 * */
static void load_balance (int this_cpu,int simulation_step)
{
    runqueue_t* this_rq=get_runqueue_cpu(this_cpu);
    int load_cpus[MAX_CPUS];
    int max_load,min_load,nr_tasks_system;
    int cpu_max_load,cpu_min_load;
    int nr_trials=0,max_trials=5;
    bool load_balanced_system=FALSE;

    int i;

    lock_rq(this_rq);

    /* If this CPU is idle -> it's always time for load balancing */
    if (this_rq->nr_runnable==0)
        this_rq->next_load_balancing=simulation_step;

    if (nr_cpus==1 || this_rq->next_load_balancing!=simulation_step) {
        unlock_rq(this_rq);
        return;
    }

    /* Attempt to get a load balance system in the number of trials allowed
    (max threads to be migrated) */
    while (!load_balanced_system && (nr_trials<max_trials)) {

        /* Compute load across CPUs, without holding remote locks */
        min_load=INT_MAX;
        max_load=-1;
        nr_tasks_system=0;

        for (i=0; i<nr_cpus; i++) {
            load_cpus[i]=get_runqueue_cpu(i)->nr_runnable;
            nr_tasks_system+=load_cpus[i];

            if (load_cpus[i]> max_load) {
                max_load=load_cpus[i];
                cpu_max_load=i;
            }

            if (load_cpus[i]< min_load) {
                min_load=load_cpus[i];
                cpu_min_load=i;
            }
        }

        if (max_load-min_load<=1) {
            load_balanced_system=TRUE;
        } else if (load_cpus[this_cpu]==max_load) {
            runqueue_t* remote_rq=get_runqueue_cpu(cpu_min_load);

            /* Adquire remote CPU's rq lock */
            double_lock_rq(this_rq,remote_rq);

            /* Make sure the thread migration actually improves load balance
             * (This is required due to concurrency issues)
             * If so move a task from the current CPU to the remote one
            */
            if ((this_rq->nr_runnable-remote_rq->nr_runnable)>1)
                move_one_task(this_rq,this_cpu,remote_rq,cpu_min_load,this_cpu,simulation_step);

            /* Release remote CPU's lock */
            unlock_rq(remote_rq);

        } else if (load_cpus[this_cpu]==min_load) {
            runqueue_t* remote_rq=get_runqueue_cpu(cpu_max_load);

            /* Adquire remote CPU's rq lock */
            double_lock_rq(this_rq,remote_rq);

            /* Make sure the thread migration actually improves load balance
             * (This is required due to concurrency issues)
             * If so move a task from the remote one CPU to the current one
            */
            if ((remote_rq->nr_runnable-this_rq->nr_runnable)>1)
                move_one_task(remote_rq,cpu_max_load,this_rq,this_cpu,this_cpu,simulation_step);

            /* Release remote CPU's lock */
            unlock_rq(remote_rq);
        }
        nr_trials++;
    }

    /* Set up next balancing period */
    this_rq->next_load_balancing+=load_balancing_period;

    unlock_rq(this_rq);
}

/* This is is the main scheduler function. It takes care of
 * selecting a new task to run and performs the associated
 * context switch if necessary.
 * */
static void schedule(int cpu,int simulation_step)
{
    task_t *prev,*next;
    runqueue_t* cpu_rq=get_runqueue_cpu(cpu);

    /* Lock the Rq*/
    lock_rq(cpu_rq);
    prev=cpu_rq->cur_task;

    if (cpu_rq->need_resched || is_idle_task(prev)) {

        /* Give current a chance to be selected again if it's still running */
        if (prev->state==TS_ONPROC)
            active_sched_class->enqueue_task(prev,cpu,1);

        next=active_sched_class->pick_next_task(cpu_rq,cpu);

        /* Pick the IDLE task if there are no runnable tasks for now*/
        if (!next) {
            next=&cpu_rq->idle_task;
            cpu_rq->cur_task=next; /* Update current as well*/
        }
        /* Perform the actual context switch */
        if (prev!=next) {

            /* Update and enqueue prev in the rq if it is still runnable */
            if (prev->state==TS_ONPROC) {
                prev->state=TS_RUNNABLE;
                prev->last_time_enqueued=simulation_step+1;
            }

            /* Change state of the new "current" task */
            next->state=TS_ONPROC;
            /* Update real time of incoming process */
            next->real_time+=(simulation_step-next->last_time_enqueued+1);
            /* Keep track of how long the task was runnable */
            add_log_register(next,TS_RUNNABLE,cpu,next->last_time_enqueued,simulation_step-next->last_time_enqueued+1);
            write_cpu_log(cpu,"(t%d): Context switch (%s)<->(%s)\n",simulation_step,prev->task_name,next->task_name);
        }

    }

    /* Clear need_resched flag */
    cpu_rq->need_resched=FALSE;

    /* Unlock the Rq*/
    unlock_rq(cpu_rq);
}

static inline int cpu_has_completed(int cpu)
{
    return (get_runqueue_cpu(cpu)->nr_runnable==0 && is_empty_slist(&sched_events[cpu]));
}

/* Update per-CPU completion status.
 * Each simulation thread must synchronize with the others
 * at the barrier twice. First, we ensure that run queues get stabilized.
 * Second, we wait for each simulation thread to update its completion status.
 * */
void synchronize_cpus(int this_cpu)
{
    if (nr_cpus==1) {
        //Single processor mode
        cpus_completed=cpu_has_completed(this_cpu);
    } else {
        // Multiprocessor mode
        /* Wait in the very first barrier --> Runqueues get stabilized !! */
        /* CPU 0 resets the cpus_completed variable */
        if (this_cpu==0)
            cpus_completed=0;

        sys_barrier_wait(&simulation_barrier);

        /* Each thread now updates its completion status in a distributed fashion */
        if (cpu_has_completed(this_cpu)) {
            pthread_mutex_lock(&simulation_lock);
            cpus_completed++;
            pthread_mutex_unlock(&simulation_lock);
        }

        sys_barrier_wait(&simulation_barrier);
    }
}

/* Main function for a per-CPU simulation thread
 * */
void* sched_cpu(void* arg)
{
    int this_cpu=(long)arg;
    int simulation_step=0;
    runqueue_t* cpu_rq=get_runqueue_cpu(this_cpu);

    /* Initial steps
     * (1) Process incomming tasks
     * (2) Call the scheduler to pick the next task the very first time
     * */

    process_sched_events(this_cpu,0);

    lock_rq(cpu_rq);
    active_sched_class->pick_next_task(cpu_rq,this_cpu);
    /* After this step the CPU may still be idle -> current may be the idle task */
    cpu_rq->cur_task->state=TS_ONPROC;
    unlock_rq(cpu_rq);

    while(cpus_completed<nr_cpus && simulation_step<max_simulation_steps) {

        /* Current runs for a tick*/
        usleep(tick_delay);

        /* Tick processing */
        scheduler_tick(this_cpu,simulation_step);

        /* Handle awoken tasks (for the next interval) */
        process_sched_events(this_cpu,simulation_step+1);

        /* load balance */
        load_balance(this_cpu,simulation_step);

        /* Select a new thread to run if necessary */
        schedule(this_cpu,simulation_step);

        /* Synchronize CPUs ...*/
        synchronize_cpus(this_cpu);

        simulation_step++;
    }

    write_cpu_log(this_cpu,"==================\n");
    return NULL;
}

/* This function enables to start up the simulator with
 * a given scheduling algorithm and specified task list.
 * */
void sched_start(slist_t* task_list, struct sched_class* sc)
{
    task_t *cur,*next;
    int task_cnt=0;
    pthread_t sim_cpu[MAX_CPUS];
    long cpu=0;
    int i=0;

    sched_init(sc);

    /* Traverse the task_list to wake up new tasks*/
    cur=head_slist(task_list);

    while(task_cnt<task_list->size) {
        cur->last_cpu=task_cnt%nr_cpus; /* Hack to automatize load balancing at the beginning */
        if (init_task_sched(cur)) {
            perror("Couldn't initialize class specific data for thread ");
            exit(1);
        }
        schedule_wake_up_new_task(cur);	/*Just inserts the sched_event in the event_queue */

        if (debug_mode)
            print_task(cur);

        cur=next_slist(task_list,cur);
        task_cnt++;
    }

    if (debug_mode) {
        printf("Scheduler initialized. Press ENTER to start simulation.\n");
        getchar();
    }

    /* Create per-CPU simulation threads */
    for (cpu=0; cpu<nr_cpus; cpu++)
        pthread_create(&sim_cpu[cpu],NULL,sched_cpu,(void*)cpu);

    /* Wait for completion of per-CPU simulation threads*/
    for (cpu=0; cpu<nr_cpus; cpu++)
        pthread_join(sim_cpu[cpu],NULL);

    /* Print log information to aid in
     * the construction of the gantt diagram
     * */
    cur=head_slist(task_list);

    for(i=0; i<task_cnt; i++) {
        print_task_log_registers(cur);
        cur=next_slist(task_list,cur);
    }

    /* Free up class-specific task data and task structures */
    cur=head_slist(task_list);

    for(i=0; i<task_cnt; i++) {
        next=next_slist(task_list,cur);
        if (active_sched_class->task_free)
            active_sched_class->task_free(cur);
        free(cur);
        cur=next;
    }

    /* Free up sched-class specific resources if necessary */
    if (active_sched_class->sched_destroy)
        active_sched_class->sched_destroy();

    printf("Simulation completed\n");
    sched_terminate(0);
}

runqueue_t* get_runqueue_cpu(int cpu)
{
    if (cpu>nr_cpus)
        return NULL;
    else
        return &runqueues[cpu];
}

static int compare_sched_event_timeouts(void *t1,void *t2)
{
    sched_event_t* e1=(sched_event_t*)t1;
    sched_event_t* e2=(sched_event_t*)t2;
    return e1->timeout-e2->timeout;
}

static inline void insert_sched_event(sched_event_t* event, int cpu)
{
    sorted_insert_slist(&sched_events[cpu], event, 1, compare_sched_event_timeouts);
}

static void put_task_to_sleep(task_t* task, int timeout)
{
    sched_event_t* event=(sched_event_t*)malloc(sizeof(sched_event_t));

    event->task=task;
    event->timeout=timeout;
    event->type=TASK_AWAKEN;

    /* Insert event in the right place of the per-cpu list*/
    insert_sched_event(event,task->last_cpu);
}

static void  schedule_wake_up_new_task(task_t* task)
{
    sched_event_t* event=(sched_event_t*)malloc(sizeof(sched_event_t));

    event->task=task;
    event->timeout=task->task_profile.arrival_time;
    event->type=NEW_TASK;

    /* Insert event in the right place of the per-cpu list*/
    insert_sched_event(event,task->last_cpu);
}
