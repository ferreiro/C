#include "sched.h"

/* Global RR quantum parameter */
int rr_quantum=3;

/* Structure to store RR thread-specific fields */
struct rr_data {
    int remaining_ticks_slice;
};

static int task_new_rr(task_t* t)
{
    struct rr_data* cs_data=malloc(sizeof(struct rr_data));

    if (!cs_data)
        return 1;  /* Cannot reserve memory */


    /* initialize the quantum */
    cs_data->remaining_ticks_slice=rr_quantum;
    t->tcs_data=cs_data;
    return 0;
}

static void task_free_rr(task_t* t)
{
    if (t->tcs_data) {
        free(t->tcs_data);
        t->tcs_data=NULL;
    }
}

static task_t* pick_next_task_rr(runqueue_t* rq,int cpu)
{
    task_t* t=head_slist(&rq->tasks);

    if (t) {
        /* Current is not on the rq*/
        remove_slist(&rq->tasks,t);
        t->on_rq=FALSE;
        rq->cur_task=t;
    }

    return t;
}

static void enqueue_task_rr(task_t* t,int cpu, int runnable)
{
    runqueue_t* rq=get_runqueue_cpu(cpu);
    struct rr_data* cs_data=(struct rr_data*) t->tcs_data;

    if (t->on_rq || is_idle_task(t))
        return;

    insert_slist(&rq->tasks,t); //Push task
    t->on_rq=TRUE;
    cs_data->remaining_ticks_slice=rr_quantum; // Reset slice

    /* If the task was not runnable before on this RQ (just changed the status)*/
    if (!runnable) {
        rq->nr_runnable++;
        t->last_cpu=cpu;
    }
}


static void task_tick_rr(runqueue_t* rq,int cpu)
{

    task_t* current=rq->cur_task;
    struct rr_data* cs_data=(struct rr_data*) current->tcs_data;

    if (is_idle_task(current))
        return;

    cs_data->remaining_ticks_slice--; /* Charge tick */

    if (cs_data->remaining_ticks_slice<=0)
        rq->need_resched=TRUE; //Force a resched !!

    if (current->runnable_ticks_left==1)
        rq->nr_runnable--; // The task is either exiting or going to sleep right now
}

static task_t* steal_task_rr(runqueue_t* rq,int cpu)
{
    task_t* t=tail_slist(&rq->tasks);

    if (t) {
        remove_slist(&rq->tasks,t);
        t->on_rq=FALSE;
        rq->nr_runnable--;
    }
    return t;
}


sched_class_t rr_sched= {
    .task_new=task_new_rr,
    .task_free=task_free_rr,
    .pick_next_task=pick_next_task_rr,
    .enqueue_task=enqueue_task_rr,
    .task_tick=task_tick_rr,
    .steal_task=steal_task_rr
};
