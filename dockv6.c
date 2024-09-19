#include "types.h"
#include "user.h"
#include "cm.h"
#include "containers.h"

// Container manager. This should communicate through pipes with client processes, so it should listen for filenames to JSON on stdin.
// When a new container is requested, it should create a set of file descriptors for stdin, stdout, and stderr, and then fork a new process.
// It should then fork a new process, change the root directory to the container's root, constrain the number of processes, and then exec the requested program.

int
main(int argc, char *argv[])
{
	if (prio_set(getpid(), PRIO_LOW)) {
		printf(2, "dockv6: failed to set priority\n");
		exit();
	}

	printf(1, "Container Manager starting...\n");
	printf(1, "CM awaiting requests...\n");

	if (strcmp(argv[1], "create") == 0 && argc == 3) {
		// Send filename to cm
		cm(argv[2]);
	}
	else {
		printf(2, "dockv6: invalid command\n");
	}

	exit();

	return 0;
}
