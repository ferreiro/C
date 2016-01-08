/*
	FCFS:
	First come first serve scheduling algorithm to be implemented.
*/
#include "sched.h"



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