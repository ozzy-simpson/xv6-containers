#include "types.h"
#include "user.h"

int
main(void)
{
	// Read container.txt to print what's inside
	int  fd, n;
	char buf[512];

	if ((fd = open("../container.txt", 0)) < 0) {
		printf(2, "\tcontainer-test: cannot open container.txt\n");
		exit();
	}

	if ((n = read(fd, buf, sizeof(buf))) < 0) {
		printf(2, "\tcontainer-test: read error\n");
		exit();
	}

	printf(1, "\tcontainer.txt = %s", buf);

	// Try to fork bomb, count how many forks until it fails
	int i = 0; // Count the number of processes
	int pid[100];
	while (1) {
		pid[i] = fork();
		if (pid[i] == 0){
			while(1);
		}
		else if (pid[i] < 0) break;
		else {
			i++;
		}
	}
	// Kill and wait for all child processes
	for (int j = 0; j < i; j++) {
		kill(pid[j]);
		wait();
	}
	i++; // Add one to account for the parent process
	printf(1, "\tmaxproc = %d\n", i);

	// Try to open ../private/container.txt
	if ((fd = open("../private/container.txt", 0)) < 0) {
		printf(1, "\tunable to open ../private/container.txt\n");
	}
	else {
		printf(1, "\table to open ../private/container.txt\n");
		close(fd);
	}

	exit();

}