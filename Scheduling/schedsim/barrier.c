#include "barrier.h"
#include <errno.h>


#ifdef POSIX_BARRIER

/* Wrapper functions to use pthread barriers */

int sys_barrier_init(sys_barrier_t* barrier, unsigned int nthreads)
{
    return pthread_barrier_init(barrier,NULL,nthreads);
}

int sys_barrier_destroy(sys_barrier_t* barrier)
{
    return pthread_barrier_destroy(barrier);
}

int sys_barrier_wait(sys_barrier_t *barrier)
{
    return pthread_barrier_wait(barrier);
}

#else

/*

Implement a synchronization barrier by using locks and condition variables. 
Complete the barrier implementation in the barrier.c file 
(functions sys_barrier_init(), sys_barrier_destroy() and sys_barrier_wait() of the #else code path). 
Activating this barrier in the scheduler entails modifying the provided Makefile so as 
to ensure that the POSIX_BARRIER preprocessor symbol is not defined.

*/

/* Barrier initialization function */
int sys_barrier_init(sys_barrier_t *barrier, unsigned int nr_threads)
{
    /* Initialize fields in sys_barrier_t
         ... To be completed ....
    */

    /*
    barrier->mutex=; // Barrier lock 
    barrier->cond=; // Condition variable where threads remain blocked
    barrier->nr_threads_arrived[X]=; // Number of threads that reached the barrier.
                                    // [0] Counter for even barriers, [1] Counter for odd barriers
    barrier->max_threads=nr_threads; // Number of threads that rely on the syncronization barrier
    barrier->cur_barrier=; // Field to indicate whether the current barrier is an even (0) or an odd (1) barrier
    */


    return 0;
}

/* Destroy barrier resources */
int sys_barrier_destroy(sys_barrier_t *barrier)
{
    /* Destroy synchronization resources associated with the barrier
          ... To be completed ....
    */
  /*
  barrier->mutex=; // Barrier lock 
  barrier->cond=; // Condition variable where threads remain blocked
  barrier->nr_threads_arrived[X]=; // Number of threads that reached the barrier.
                                  // [0] Counter for even barriers, [1] Counter for odd barriers
  barrier->max_threads=nr_threads; // Number of threads that rely on the syncronization barrier
  barrier->cur_barrier=; // Field to indicate whether the current barrier is an even (0) or an odd (1) barrier
  */

    return 0;
}

/* Main synchronization operation */
int sys_barrier_wait(sys_barrier_t *barrier)
{
    /* Implementation outline:
       - Every thread arriving at the barrier adquires the lock and increments the nr_threads_arrived
        counter atomically
         * In the event this is not the last thread to arrive at the barrier, the thread
           must block in the condition variable
         * Otherwise...
            1. Reset the barrier state in preparation for the next invocation of sys_barrier_wait() and
            2. Wake up all threads blocked in the barrier
       - Don't forget to release the lock before returning from the function

        ... To be completed ....
    */
    /*
    barrier->mutex //  adquires the lock
    // increments the nr_threads_arrived counter atomically
    nr_threads_arrived
    */
    
    return 0;
}

#endif /* POSIX_BARRIER */
