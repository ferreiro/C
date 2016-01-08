/*
	PRIO:
	Preemtive priority based algorithm.
*/
#include "sched.h"



// Instancing the interface of operations for the 
// PRIO scheduler as follows:
sched_class_t prio_sched={
    .task_new=task_new_prio,
	.task_free=task_free_prio,
	.pick_next_task=pick_next_task_prio,
	.enqueue_task=enqueue_task_prio,
	.task_tick=task_tick_prio,
	.steal_task=steal_task_prio
};