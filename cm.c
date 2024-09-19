#include "types.h"
#include "user.h"
#include "jsmn.h"
#include "cm.h"

// Container manager. This should communicate through pipes with client processes, so it should listen for filenames to JSON on stdin.
// When a new container is requested, it should create a set of file descriptors for stdin, stdout, and stderr, and then fork a new process.
// It should then fork a new process, change the root directory to the container's root, constrain the number of processes, and then exec the requested program.

void
cm(char *spec)
{
	int  n, fd, pid;
	char json[512];

	// Open JSON file
	if ((fd = open(spec, 0)) < 0) {
		printf(2, "cm: cannot open %s\n", spec);
		exit();
	}

	// Read JSON file
	if ((n = read(fd, json, sizeof(json))) < 0) {
		printf(2, "cm: read error\n");
		exit();
	}

	// Parse and print JSON
	jsmn_parser parser;
	jsmntok_t   tokens[10];

	jsmn_init(&parser);
	if ((n = jsmn_parse(&parser, json, strlen(json), tokens, 10)) < 0) {
		printf(2, "cm: parse error\n");
		exit();
	}

	// Determine init
	char init[128]; // buffer to hold init value
	strncpy(init, json + tokens[2].start,
			tokens[2].end - tokens[2].start);     // copy init value from json to buffer
	init[tokens[2].end - tokens[2].start] = '\0'; // null terminate buffer

	// Determine root
	char root[128]; // buffer to hold root value
	strncpy(root, json + tokens[4].start,
			tokens[4].end - tokens[4].start);     // copy root value from json to buffer
	root[tokens[4].end - tokens[4].start] = '\0'; // null terminate buffer

	// Determine max processes
	char maxproc[128]; // buffer to hold max proc value
	strncpy(maxproc, json + tokens[6].start,
			tokens[6].end - tokens[6].start);        // copy max proc value from json to buffer
	maxproc[tokens[6].end - tokens[6].start] = '\0'; // null terminate buffer
	int procs                                = atoi(maxproc);

	printf(1, "CM creating container with init = %s, root fs = %s, and max num processes = %d\n", init,
			root, procs);

	// Close JSON file
	close(fd);

	// Fork new process
	pid = fork();
	if (pid == 0) {
		// Child process

		// Enter container
		if (cm_create_and_enter() < 0) {
			printf(2, "cm: enter error\n");
			exit();
		}

		// Change root directory to container's root
		if (cm_setroot(root, strlen(root)) < 0) {
			printf(2, "cm: setroot error\n");
			exit();
		}

		// Constrain number of processes
		if (cm_maxproc(procs) < 0) {
			printf(2, "cm: maxproc error\n");
			exit();
		}

		// Exec requested program
		char *argv[] = {init, 0};
		exec(init, argv);
		printf(2, "cm: exec %s failed\n", init);
		exit();
	}
	wait();
}
