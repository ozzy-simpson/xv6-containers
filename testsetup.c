#include "testsetup.h"

// This program creates the filesystem for the containers so that containers can run the shell and other programs

void
testsetup(int print)
{
	for (int i = 0; i < NUMCONTAINERS; i++) {
		if (print) printf(1, "Creating container %d filesystem\n", i);
		// Create a directory for the container, named c<i>
		char dirname[3];
		dirname[0] = 'c';
		dirname[1] = '0' + i;
		dirname[2] = '\0';
		if (mkdir(dirname) == -1) {
			printf(2, "mkdir failed\n");
			exit();
		}

		// Copy files from the host to the container
		char *files[] = {"cat", "containertest", "echo", "grep", "kill", "ln", "ls", "mkdir", "rm", "sh", "shc", "shmtest", "synctest"};
		for (int j = 0; j < 13; j++) {
			// Copy file using link
			char src[30]; // will be original filename to copy
			char dst[40]; // will be c<i>/<filename>
			strcpy(src, files[j]);
			strcpy(dst, dirname); // Copy dirname to dst

			// Append filename to dst, with a / in between
			int len  = strlen(dst);
			dst[len] = '/';
			strcpy(dst + len + 1, src);

			if (link(src, dst) == -1) {
				printf(2, "link failed\n");
				exit();
			}

			if (print) printf(1, "\t Copied %s\n", src);
		}

		// Create txt file for the container that contains the container number
		char txtname[30];
		strcpy(txtname, dirname);
		int len      = strlen(txtname);
		txtname[len] = '/';
		strcpy(txtname + len + 1, "container.txt");

		// Create/open file
		if (print) printf(1, "\t Creating %s\n", txtname);
		int fd = open(txtname, O_CREATE | O_RDWR);
		if (fd == -1) {
			printf(2, "open failed\n");
			exit();
		}

		// Write container number to file
		char buf[2];
		buf[0] = '0' + i;
		buf[1] = '\n';
		if (write(fd, buf, 2) != 2) {
			printf(2, "write failed\n");
			exit();
		}

		// Close file
		if (close(fd) == -1) {
			printf(2, "close failed\n");
			exit();
		}
	}

	// Create a sub-directory in c0, with container.txt containing 0
	if (print) printf(1, "Creating subdirectory in c0\n");
	if (mkdir("/c0/private") == -1) {
		printf(2, "mkdir failed\n");
		exit();
	}
	int fd = open("/c0/private/container.txt", O_CREATE | O_RDWR);
	if (fd == -1) {
		printf(2, "open failed\n");
		exit();
	}
	char buf[2];
	buf[0] = '0';
	buf[1] = '\n';
	if (write(fd, buf, 2) != 2) {
		printf(2, "write failed\n");
		exit();
	}
	if (close(fd) == -1) {
		printf(2, "close failed\n");
		exit();
	}

	// Remove shc from /
	if (print) printf(1, "Removing shc from /\n");
	if (unlink("/shc") == -1) {
		printf(2, "unlink failed\n");
		exit();
	}
}
