#include "types.h"
#include "user.h"
#include "containers.h"

int
main(void)
{
	int result;

	printf(1, "Starting to test Module 4: Scheduling\n");
	printf(1, "-------------------------------------\n");

	// Level 0: System call implemented, priorities get set
	result = prio_set(getpid(), 0);
	if (result == 0) {
		printf(1, "Level 0: System call implemented, priority set: OK\n");
	} else {
		printf(1, "Level 0: System call implemented, priority set: FAILED\n");
	}
	
	printf(1, "-------------------------------------\n");

	result = prio_set(getpid(), -1);
	if (result == 0) {
		printf(1, "Level 1: Out of bounds priority (negative): OK\n");
	} else {
		printf(1, "Level 1: Out of bounds priority (negative): FAILED\n");
	}

	prio_set(getpid(), 1);
	result = prio_set(getpid(), 0);
	if (result == -1) {
		printf(1, "Level 1: Lower priority than current process: OK\n");
	} else {
		printf(1, "Level 1: Lower priority than current process: FAILED\n");
	}

	// At this point, curproc->priority should be 1

	// Test that a priority cannot be set by a descendant
	int parent = getpid();
	int pid = fork();
	if (pid == 0) {
		result = prio_set(parent, 0);
		if (result == -1) {
			printf(1, "Level 1: Descendant process setting priority of ancestor: OK\n");
		} else {
			printf(1, "Level 1: Descendant process setting priority of parent: FAILED\n");
		}
		exit();
	}
	else {
		wait();
	}
	
	// At this point, curproc->priority should still be 1
	prio_set(getpid(), 1);

	// Test that a priority can be set by an ancestor
	pid = fork();
	if (pid == 0) {
		// Spin endlessly (until killed by parent)
		while(1)
			;

		exit();
	}
	else {
		result = prio_set(pid, 5);
		if (result == 0) {
			printf(1, "Level 1: Ancestor process setting priority of descendant: OK\n");
		}
		else {
			printf(1, "Level 1: Ancestor process setting priority of descendant: FAILED\n");
		}

		kill(pid); // Kill the child process

		wait();
	}

	result = prio_set(getpid(), PRIO_MAX + 1);
	if (result == 0) {
		printf(1, "Level 1: Out of bounds priority (lower than max): OK\n");
	} else {
		printf(1, "Level 1: Out of bounds priority (lower than max): FAILED\n");
	}
	printf(1, "-------------------------------------\n");

	exit();
	return 0;
}