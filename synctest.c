#include "types.h"
#include "user.h"

int
main(void)
{	
	// Read container.txt to get container number
	int  fd, n;
	char buf[512];

	if ((fd = open("../container.txt", 0)) < 0) {
		printf(2, "Synchronization: cannot open container.txt: FAILED\n");
		exit();
	}

	if ((n = read(fd, buf, sizeof(buf))) < 0) {
		printf(2, "Synchronization: read container.txt error: FAILED\n");
		exit();
	}
	
	// Make mutex, print out ID
	int lockid = mutex_create("Test");
	if (lockid < 0) {
		printf(1, "Synchronization: mutex_create: FAILED\n");
		exit();
	}
	
	// Print ID
	printf(1, "Synchronization: ID of mutex inside container %c: %d\n", buf[0], lockid);

	exit();
}
