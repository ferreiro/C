/*
	PRIO:
	Preemptive priority-based scheduler (PRIO).

	This scheduler will make decisions based on each task’s static priority value 
	defined in the input file passed as a command- line argument to the simulator (-i option). 
	Students are strongly encouraged to pay spEcial attention to the implementation of 
	---> the SJF4 policy, because it is somewhat similar to that of the priority-based scheduler.
*/
#include "sched.h"

/*
	This scheduler will make decisions based on each task’s static 
	priority value defined in the input file passed as a command-line
	argument to the simulator 

	Tenemos que tener una lista de tareas ordenadas de mayor 
	a menor prioridad (cuanto más bajo sea el número, mayor prioridad tenemos)
*/
static task_t* pick_next_task_prio(runqueue_t* rq,int cpu)
{
	task_t* t=head_slist(&rq->tasks); //List sorted by Task Priority (just pick the first one with more priority)

    if (t) {
        /* Current is not on the rq*/
        remove_slist(&rq->tasks,t);
        t->on_rq=FALSE;
        rq->cur_task=t;
    }

    return t;
}


static int compare_tasks_cpu_priority(void *t1,void *t2)
{
    task_t* tsk1=(task_t*)t1;
    task_t* tsk2=(task_t*)t2;
    return tsk1->prio-tsk2->prio;
}

static void enqueue_task_prio(task_t* t,int cpu, int runnable)
{
	runqueue_t* rq=get_runqueue_cpu(cpu);

	if (t->on_rq || is_idle_task(t))
	    return;


	if (t->flags & TF_INSERT_FRONT) {
	    //Clear flag
	    t->flags&=~TF_INSERT_FRONT;
	    sorted_insert_slist_front(&rq->tasks, t, 1, compare_tasks_cpu_priority);  //Push task
	} else
	    sorted_insert_slist(&rq->tasks, t, 1, compare_tasks_cpu_priority);  //Push task

	t->on_rq=TRUE;

	/* If the task was not runnable before, update the number of runnable tasks in the rq*/
	if (!runnable) {
	    rq->nr_runnable++;
	    t->last_cpu=cpu;
	}

}

/*
	Gets invoked every tick of the simulation (each iteration of the main loop). 
	When the currently running task on the CPU is about to complete its current 
	CPU burst (one tick left), this function decrements the number of runnable tasks 
	in the run queue (nr_runnable field).
*/
static void task_tick_prio(runqueue_t* rq,int cpu)
{
	task_t* current=rq->cur_task;

	if (is_idle_task(current))
	    return;

	if (current->runnable_ticks_left==1)
	    rq->nr_runnable--; // The task is either exiting or going to sleep right now
} 

/*
	STEALING
	In the event the stealing operation of the scheduling algoritm gets invoked, 
	this scheduler will select the lowest priority task found in the run queue. 
*/
static task_t* steal_task_fcfs(runqueue_t* rq,int cpu)
{	

	// Due our runqueue is ordered from most priority (left)
	// to lower priority (right). We only need to return the last
	// element of the runqueue

	task_t* t=tail_slist(&rq->tasks); // returns the last element of thelist (butitdoesnotremoveitfromthelist).

	if (t) {
		remove_slist(&rq->tasks,t);
		t->on_rq=FALSE; // Now thread is not on the runqueue
		rq->nr_runnable--; // Decrease number of threads runnable on runqueue
	}

	return t;
}

// Instancing the interface of operations for the 
// PRIO scheduler as follows:
sched_class_t prio_sched={
	.pick_next_task=pick_next_task_prio,
	.enqueue_task=enqueue_task_prio,
	.task_tick=task_tick_prio,
	.steal_task=steal_task_prio
};