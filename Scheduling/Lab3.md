
### PROCESS:
Creating a new process is used with fork().
- **Fork()** copy memory image from parent to the new child process.
- Parent and child process don't share memory. 
- Example:
	
	#include <unistd.h>
	pid_t fork(void);

	int main () {
		pid_t child_pid;
		child_pid = fork ();
		if (child_pid != 0) {
			// This code is executed by the PARENT process ONLY
			.....
		else {
	 		//  This code is executed by the CHILD process ONLY
		..... 
		}
		// BOTH the parent and the child will run this code
		// (unless the process invoked exec, exit, or returned from the main function.)
	}

Normally the child process wants to execute a **different program** from the parent. So we can use a funcion from exec() to override the memory image (where the program code is stored).

We **terminate a process** using the return in main function. But, we can also finish it before:

	#include <stdlib.h>
	void exit(int status);

For the parent process waits until the child finishes, we can use the wait() function.

	#include <sys/wait.h>
	pid_t wait(int *stat_loc);
	pid_t waitpid(pid_t pid, int *stat_loc, int options);

	
