#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "cm.h"

void
sharedmemory(void)
{
	int pid;

	// Make shared memory inside and outside containers → show that one not accessible from the other
	// Create shared memory outside container
	char *test = shm_get("Test");
	if (!test) {
		printf(1, "Shared Memory: shm_get: FAILED\n");
		exit();
	}

	// Print address of shared memory
	// Should be accessible outside of containers 
	printf(1, "Shared Memory: address of shared memory outside container: %p\n", shm_retaddr("Test"));

	// Make shared memory across two containers with the same name, show that addresses are different
	pid = fork();
	if (pid < 0) {
		printf(1, "Shared Memory: fork: FAILED\n");
		exit();

	}
	else if (pid == 0) { 
		cm("c0-1.json");

		exit();
	}

	// Wait for first container to finish
	wait();

	// Make shared memory inside second container
	pid = fork();
	if (pid < 0) {
		printf(1, "Shared Memory: fork: FAILED\n");
		exit();
	}
	else if (pid == 0) {
		cm("c1-1.json");

		exit();
	}

	// Wait for second container to finish
	wait();
}

void
synchronization(void)
{
	int pid;

	// Make mutx inside and outside containers → show that one not accessible from the other
	// Create mutex outside container
	int lockid = mutex_create("Test");
	if (lockid < 0) {
		printf(1, "Synchronization: mutex_create FAILED\n");
		exit();
	}

	// Print mutex id
	// Should be accessible outside of containers 
	printf(1, "Synchronization: ID of mutex outside container: %d\n", lockid);

	// Make mutex across two containers with the same name, show that IDs are different
	pid = fork();
	if (pid < 0) {
		printf(1, "Synchronization: fork FAILED\n");
		exit();
	}
	else if (pid == 0) { 
		cm("c0-2.json");

		exit();
	}

	// Wait for first container to finish
	wait();

	// Make mutex inside second container
	pid = fork();
	if (pid < 0) {
		printf(1, "Synchronization: fork FAILED\n");
		exit();
	}
	else if (pid == 0) {
		cm("c1-2.json");

		exit();
	}

	// Wait for second container to finish
	wait();
}

int
main(void)
{
	printf(1, "Starting to test integration\n");
	printf(1, "-------------------------------------\n");

	// Check if `testsetup` was already run, if not, run it
	int fd = open("c0", O_RDONLY);
	if (fd == -1) {
		printf(1, "Test environment not properly set up. Run `setup` before running these tests\n");
		exit();
	}

	// Shared memory
	sharedmemory();

	printf(1, "-------------------------------------\n");

	// Synchronization
	synchronization();

	printf(1, "-------------------------------------\n");

	exit();
	return 0;
}