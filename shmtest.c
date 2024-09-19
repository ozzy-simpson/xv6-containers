#include "types.h"
#include "user.h"

int
main(void)
{	
	// Read container.txt to get container number
	int  fd, n;
	char buf[512];

	if ((fd = open("../container.txt", 0)) < 0) {
		printf(2, "Shared Memory: cannot open container.txt: FAILED\n");
		exit();
	}

	if ((n = read(fd, buf, sizeof(buf))) < 0) {
		printf(2, "Shared Memory: read container.txt error: FAILED\n");
		exit();
	}
	
	// Make shared memory, print out address
	char *test = shm_get("Test");
	if (!test) {
		printf(1, "Shared Memory: shm_get: FAILED\n");
		exit();
	}
	
	// Print address
	void *addr = shm_retaddr("Test");
	printf(1, "Shared Memory: address of shared memory inside container %c: %p\n", buf[0], addr);

	exit();
}