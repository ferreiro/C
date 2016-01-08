/*
	FCFS:
	First come first serve scheduling algorithm to be implemented.
*/
#include "sched.h"


/*
Remove the first taks on the runqueue

Will be invoked to select a new task to run on a CPU. 
This function will pick one task from the task list of the run queue, 
will then remove the selected task from the list and will return the selected task. 
Obviously, the mechanism to select a task is specific to each scheduling algorithm.
*/
static task_t* pick_next_task_fcfs(runqueue_t* rq,int cpu)
{
	task_t* t=head_slist(&rq->tasks); // Get head task from runque

	if (t) {
	    /* Current is not on the rq*/
	    remove_slist(&rq->tasks,t);
	    t->on_rq=FALSE; // this task is not on runqueue. Is going to be executed
	    rq->cur_task=t; // Pointer to the task in the CPU. It may be the idle task
	}

	return t;
}

/*

In this algorithm, the new tasks added will be 
push to the end of the runqueue (IO operations, or enter system or awakes...)

Gets invoked every time that a task must be enqueued on a given run queue. 
Three situations may trigger the invocation of this function 
	(1) a task enters the system, 
	(2) a task awakes upon completion of an I/O operation and 
	(3) the task was mi- grated onto this CPU from a different CPU.
*/
static void enqueue_task_fcfs(task_t* t,int cpu, int runnable)
{
	runqueue_t* rq=get_runqueue_cpu(cpu);

	if (t->on_rq || is_idle_task(t))
	    return; // La tarea estaba ya o es idle... No hacer nada

	insert_slist(&rq->tasks,t); //Push task to the end of the runqueu
	t->on_rq=TRUE;

	/* If the task was not runnable before on this RQ (just changed the status)*/
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
static void task_tick_fcfs(runqueue_t* rq,int cpu)
{
	task_t* current=rq->cur_task;

	if (is_idle_task(current))
	    return;

	if (current->runnable_ticks_left==1)
	    rq->nr_runnable--; // The task is either exiting or going to sleep right now
} 


/*
	STEALING
	When the stealing operation of the scheduling algoritm gets invoked, 
	by convention, FCFS will pick the last task in the run queue.
*/
static task_t* steal_task_fcfs(runqueue_t* rq,int cpu)
{	
	task_t* t=tail_slist(&rq->tasks); // returns the last element of thelist (butitdoesnotremoveitfromthelist).

	if (t) {
		remove_slist(&rq->tasks,t);
		t->on_rq=FALSE; // Now thread is not on the runqueue
		rq->nr_runnable--; // Decrease number of threads runnable on runqueue
	}

	return t;
}

// Instancing the interface of operations for the 
// FCFS scheduler as follows:
sched_class_t fcfs_sched={
	.pick_next_task=pick_next_task_fcfs,
	.enqueue_task=enqueue_task_fcfs,
	.task_tick=task_tick_fcfs,
	.steal_task=steal_task_fcfs
};