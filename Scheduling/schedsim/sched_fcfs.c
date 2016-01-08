/*
	FCFS:
	First come first serve scheduling algorithm to be implemented.
*/
#include "sched.h"


static task_t* pick_next_task_fcfs(runqueue_t* rq,int cpu)
{
	return NULL;
}

static void enqueue_task_fcfs(task_t* t,int cpu, int runnable)
{

}

static void task_tick_fcfs(runqueue_t* rq,int cpu)
{

} 

// STEALING
// When the stealing operation of the scheduling algoritm gets invoked, 
// by convention, FCFS will pick the last task in the run queue.

static task_t* steal_task_fcfs(runqueue_t* rq,int cpu)
{	
	return NULL;
}

// Instancing the interface of operations for the 
// FCFS scheduler as follows:
sched_class_t fcfs_sched={
    .task_new=task_new_fcfs,
	.task_free=task_free_fcfs,
	.pick_next_task=pick_next_task_fcfs,
	.enqueue_task=enqueue_task_fcfs,
	.task_tick=task_tick_fcfs,
	.steal_task=steal_task_fcfs
};