/*
	PRIO:
	Preemptive priority-based scheduler (PRIO).

	This scheduler will make decisions based on each task’s static priority value 
	defined in the input file passed as a command- line argument to the simulator (-i option). 
	Students are strongly encouraged to pay spEcial attention to the implementation of 
	---> the SJF4 policy, because it is somewhat similar to that of the priority-based scheduler.
*/
#include "sched.h"


static task_t* pick_next_task_prio(runqueue_t* rq,int cpu)
{
	return NULL;
}

static void enqueue_task_prio(task_t* t,int cpu, int runnable)
{

}

static void task_tick_prio(runqueue_t* rq,int cpu)
{

} 

// STEALING
// In the event the stealing operation of the scheduling algoritm gets invoked, 
// this scheduler will select the lowest priority task found in the run queue. 
static task_t* steal_task_prio(runqueue_t* rq,int cpu)
{	
	return NULL;
}

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