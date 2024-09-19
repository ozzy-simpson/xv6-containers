#include "types.h"
#include "user.h"
#include "containers.h"
#include "fcntl.h"
#include "testsetup.h"
#include "cm.h"

struct pathTest {
	char *path; // Directory path to chdir to
	char *file; // Expected file in that directory
};

void
die(char *s)
{
	printf(2, "%s\n", s);
	exit();
}

void
level0(void)
{
	int pid;

	// Try going into a container and running container-test
	printf(1, "Level 0: creating container with c0-0.json.\n\t");
	pid = fork();
	if (pid == 0) {
		cm("c0-0.json");

		exit();
	} else {
		wait();
	}
}

void
level1(void)
{
	int result, pid, fd;
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		// Change root
		result = cm_setroot("/c0/", sizeof("/c0/"));
		if (result == -1) die("Level 1: cm_setroot: FAILED");

		// Check if container.txt exists and contains the correct container number
		// Open container
		int fd = open("container.txt", O_RDONLY);
		if (fd == -1) die("Level 1: cm_setroot: FAILED");

		// Read container.txt
		char buf[1];
		int  n = read(fd, buf, sizeof(buf));
		if (n != 1) {
			close(fd);
			die("Level 1: cm_setroot: FAILED");
		}

		// Check if container.txt contains the correct container number
		if (buf[0] != '0') {
			// Container number is incorrect
			close(fd);
			die("Level 1: set root = /c0/: FAILED");
		}
		close(fd);

		printf(1, "Level 1: set root = /c0/: PASSED\n");

		exit();
	}
	else {
		wait();
	}

	// Double root set
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		// Change root
		cm_setroot("/c0/", sizeof("/c1/"));
		result = cm_setroot("/", sizeof("/"));
		if (result == -1) {
			printf(1, "Level 1: double root set: PASSED\n");
		}
		else die("Level 1: double root set: FAILED");

		exit();
	}
	else {
		wait();
	}

	// Switch to non-directory
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		// Change root
		result = cm_setroot("/container-spec.json", sizeof("/container-spec.json"));
		if (result == -1) {
			printf(1, "Level 1: invalid path (non-directory): PASSED\n");
		}
		else die("Level 1: invalid path (non-directory): FAILED");

		exit();
	}
	else {
		wait();
	}

	// Switch to non-existent directory
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		// Change root
		result = cm_setroot("/c/", sizeof("/c/"));
		if (result == -1) {
			printf(1, "Level 1: invalid path (directory): PASSED\n");
		}
		else die("Level 1: invalid path (directory): FAILED");

		exit();
	}
	else {
		wait();
	}

	// Try to go out of root
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		// Change root
		result = cm_setroot("/c0/", sizeof("/c0/"));
		if (result != 0) die("Level 1: cm_setroot: FAILED");

		// Try to go out of root using chdir
		result = chdir(".."); // We should stay in /c0/ after this
		if (result == -1) die("Level 1: chdir: FAILED");

		// Check if container.txt exists and contains the correct container number
		// Open container
		int fd = open("container.txt", O_RDONLY);
		if (fd == -1) die("Level 1: root enforced: FAILED");

		// Read container.txt
		char buf[1];
		int  n = read(fd, buf, sizeof(buf));
		if (n != 1) {
			close(fd);
			die("Level 1: root enforced: FAILED");
		}

		// Check if container.txt contains the correct container number
		if (buf[0] != '0') {
			// Container number is incorrect
			close(fd);
			die("Level 1: root enforced (cd ..): FAILED");
		}
		close(fd);

		printf(1, "Level 1: root enforced (cd ..): PASSED\n");

		exit();
	} else {
		wait();
	}

	// Try to open file outside of root
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		// Change root
		result = cm_setroot("/c0/", sizeof("/c0/"));
		if (result != 0) die("Level 1: cm_setroot: FAILED");

		// Try opening file outside of root
		int fd = open("../helloworld.txt", O_RDONLY);
		if (fd == -1) {
			printf(1, "Level 1: root enforced (open): PASSED\n");
		} else {
			// Close file
			close(fd);
			die("Level 1: root enforced (open): FAILED");
		}

		exit();
	} else {
		wait();
	}

	// More tests of chdir with variations of ../ and ./
	struct pathTest tests[] = {
	  {".", "container.txt"},
	  {"./", "container.txt"},
	  {"..", "container.txt"},
	  {"../", "container.txt"},
	  {"../..", "container.txt"},
	  {"../../", "container.txt"},
	  {"../../.", "container.txt"},
	  {".", "container.txt"},
	  {"./../", "container.txt"},
	  {"../private/", "container.txt"},
	  {"../../private", "container.txt"}
	};

	// Test each test variation
	int numTests = sizeof(tests) / sizeof(tests[0]);
	for (int i = 0; i < numTests; i++) {
		pid = fork();
		if (pid < 0) die("Level 1: fork: FAILED");
		else if (pid == 0) {
			result = cm_setroot("/c0/", sizeof("/c0/"));
			if (result != 0) die("Level 1: cm_setroot: FAILED");

			// Try to move using chdir
			result = chdir(tests[i].path);
			if (result == -1) {
				printf(2, "Level 1: chdir %s: FAILED\n", tests[i].path);
				exit();
			}

			// Check if file exists and contains the correct container number
			// Open container
			int fd = open(tests[i].file, O_RDONLY);
			if (fd == -1) die("Level 1: root enforced: FAILED");

			// Read container.txt
			char buf[1];
			int  n = read(fd, buf, sizeof(buf));
			if (n != 1) {
				close(fd);
				die("Level 1: root enforced: FAILED");
			}

			// Check if container.txt contains the correct container number
			if (buf[0] != '0') {
				// Container number is incorrect
				close(fd);
				printf(2, "Level 1: root enforced (cd %s): FAILED\n", tests[i].path);
				exit();
			}
			close(fd);

			printf(1, "Level 1: root enforced (cd %s): PASSED\n", tests[i].path);

			exit();
		} else {
			wait();
		}
	}

	// Ensure that root is inherited by children
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		result = cm_setroot("/c0/", sizeof("/c0/"));
		if (result != 0) die("Level 1: cm_setroot: FAILED");

		// Try to change root in child
		int pid2 = fork();
		if (pid2 < 0) die("Level 1: fork: FAILED");
		else if (pid2 == 0) {
			// Check if we're in c0
			// Check if container.txt exists and contains the correct container number
			// Open container
			int fd = open("container.txt", O_RDONLY);
			if (fd == -1) die("Level 1: cm_setroot: FAILED");

			// Read container.txt
			char buf[1];
			int  n = read(fd, buf, sizeof(buf));
			if (n != 1) {
				close(fd);
				die("Level 1: cm_setroot: FAILED");
			}

			// Check if container.txt contains the correct container number
			if (buf[0] != '0') {
				// Container number is incorrect
				die("Level 1: root inherited by children: FAILED");
			} else {
				printf(1, "Level 1: root inherited by children: PASSED\n");
			}
			close(fd);

			exit();
		} else {
			wait();
		}
		exit();
	} else {
		wait();
	}

	// Ensure that child cannot set root if parent already set root
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		result = cm_setroot("/c0/", sizeof("/c0/"));
		if (result != 0) die("Level 1: cm_setroot: FAILED");

		// Try to change root in child
		int pid2 = fork();
		if (pid2 < 0) die("Level 1: fork: FAILED");
		else if (pid2 == 0) {
			// Try to change root in child
			result = cm_setroot("/c1/", sizeof("/c1/"));
			if (result == -1) {
				printf(1, "Level 1: root cannot be set by child, where parent already set: PASSED\n");
			} else {
				die("Level 1: root cannot be set by child, where parent already set: FAILED");
			}
			exit();
		} else {
			wait();
		}
		exit();
	} else {
		wait();
	}

	// Ensure that child cannot chdir out of root
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		result = cm_setroot("/c0/", sizeof("/c0/"));
		if (result != 0) die("Level 1: cm_setroot: FAILED");

		// Try to change root in child
		int pid2 = fork();
		if (pid2 < 0) die("Level 1: fork: FAILED");
		else if (pid2 == 0) {
			// Try to go out of root
			result = chdir(".."); // We should stay in /c0/ after this
			if (result == -1) die("Level 1: chdir: FAILED");

			// Check if container.txt exists and contains the correct container number
			// Open container
			fd = open("container.txt", O_RDONLY);
			if (fd == -1) die("Level 1: root enforced: FAILED");

			// Read container.txt
			char buf[1];
			int  n = read(fd, buf, sizeof(buf));
			if (n != 1) {
				close(fd);
				die("Level 1: root enforced: FAILED");
			}

			// Check if container.txt contains the correct container number
			if (buf[0] != '0') {
				// Container number is incorrect
				die("Level 1: root enforced in child (cd ..): FAILED");
			} else {
				printf(1, "Level 1: root enforced in child (cd ..): PASSED\n");
			}
			close(fd);

			exit();
		} else {
			wait();
		}
		exit();
	} else {
		wait();
	}

	// Ensure that child cannot open file outside of root
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		result = cm_setroot("/c0/", sizeof("/c0/"));
		if (result != 0) die("Level 1: cm_setroot: FAILED");

		// Try to change root in child
		int pid2 = fork();
		if (pid2 < 0) die("Level 1: fork: FAILED");
		else if (pid2 == 0) {
			// Try opening file outside of root
			fd = open("../helloworld.txt", O_RDONLY);
			if (fd == -1) {
				printf(1, "Level 1: root enforced in child (open): PASSED\n");
			} else {
				// Close file
				close(fd);
				die("Level 1: root enforced in child (open): FAILED");
			}

			exit();
		} else {
			wait();
		}
		exit();
	} else {
		wait();
	}

	int count_successes = 0;

	// Test other file operations: exec
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		result = cm_setroot("/c0/", sizeof("/c0/"));
		if (result != 0) die("Level 1: cm_setroot: FAILED");

		// Try to execute file inside root
		char *argv2[] = {"../echo", "Level 1: root enforced (exec) 1: PASSED", 0};
		exec(argv2[0], argv2);

		// Will only get here if exec fails
		die("Level 1: root enforced (exec): FAILED");
	} else {
		wait();
	}

	// Test other file operations: exec
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		result = cm_setroot("/c0/", sizeof("/c0/"));
		if (result != 0) die("Level 1: cm_setroot: FAILED");

		// Try to execute file outside of root (which does not exist in /, only in containers)
		char *argv[] = {"../shc", 0};
		exec(argv[0], argv);

		// Will only get here if exec fails
		die("Level 1: root enforced (exec) 2: FAILED");
	} else {
		kill(pid);
		wait();
	}

	// Test other file operations: link
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		count_successes = 0;
		result          = cm_setroot("/c0/", sizeof("/c0/"));
		if (result != 0) die("Level 1: cm_setroot: FAILED");

		// Try to link outside of root
		if (link("../container.txt", "test1.txt") == 0) {
			count_successes++;
			if (unlink("test1.txt") == -1) die("Level 1: unlink: FAILED");
		}

		// Try to link file inside root
		if (link("container.txt", "test2.txt") == 0) {
			count_successes++;
			if (unlink("test2.txt") == -1) die("Level 1: unlink: FAILED");
		}

		if (count_successes == 2) {
			printf(1, "Level 1: root enforced (link): PASSED\n");
		} else {
			die("Level 1: root enforced (link): FAILED");
		}

		exit();
	} else {
		wait();
	}

	// Test other file operations: unlink
	fd = open("helloworld.txt", O_CREATE | O_RDWR);
	if (fd == -1) die("Level 1: file create: FAILED");
	close(fd);
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		count_successes = 0;
		result          = cm_setroot("/c0/", sizeof("/c0/"));
		if (result != 0) die("Level 1: cm_setroot: FAILED");

		// Try to unlink outside of root
		if (unlink("../helloworld.txt") == -1) count_successes++;

		// Create test file in root
		int fd = open("test.txt", O_CREATE | O_RDWR);
		if (fd == -1) die("Level 1: file create: FAILED");
		close(fd);

		// Try to unlink file inside root
		if (unlink("test.txt") == 0) count_successes++;

		// Create test file in root
		fd = open("../test.txt", O_CREATE | O_RDWR); // should create file in set root
		if (fd == -1) die("Level 1: file create: FAILED");
		close(fd);

		// Try to unlink file inside root
		if (unlink("../test.txt") == 0) count_successes++;

		if (count_successes == 3) {
			printf(1, "Level 1: root enforced (unlink): PASSED\n");
		} else {
			die("Level 1: root enforced (unlink): FAILED");
		}

		exit();
	} else {
		wait();
	}

	// Test other file operations: mkdir
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		count_successes = 0;
		result          = cm_setroot("/c0/", sizeof("/c0/"));
		if (result != 0) die("Level 1: cm_setroot: FAILED");

		// Try to mkdir outside of root (should just make a directory in currently set root)
		if (mkdir("../container") == 0) {
			count_successes++;
			if (unlink("container") == -1) die("Level 1: unlink: FAILED");
		}

		// Try to execute file inside root
		if (mkdir("container") == 0) {
			count_successes++;
			if (unlink("container") == -1) die("Level 1: unlink: FAILED");
		}

		if (count_successes == 2) {
			printf(1, "Level 1: root enforced (mkdir): PASSED\n");
		} else {
			die("Level 1: root enforced (mkdir): FAILED");
		}

		exit();
	} else {
		wait();
	}

	// Test other file operations: mknod
	pid = fork();
	if (pid < 0) die("Level 1: fork: FAILED");
	else if (pid == 0) {
		count_successes = 0;
		result          = cm_setroot("/c0/", sizeof("/c0/"));
		if (result != 0) die("Level 1: cm_setroot: FAILED");

		// Try to mknod outside of root
		if (mknod("../container.dev", 0, 0) == 0) {
			count_successes++;
			if (unlink("container.dev") == -1) die("Level 1: unlink: FAILED");
		}

		// Try to mknod inside root
		if (mknod("container.dev", 0, 0) == 0) {
			count_successes++;
			if (unlink("container.dev") == -1) die("Level 1: unlink: FAILED");
		}

		if (count_successes == 2) {
			printf(1, "Level 1: root enforced (mknod): PASSED\n");
		} else {
			die("Level 1: root enforced (mknod): FAILED");
		}

		exit();
	} else {
		wait();
	}
}

void
level2(void)
{
	int result, pid;
	// Test that maxproc has to be set within a container
	result = cm_maxproc(1);
	if (result == -1) {
		printf(1, "Level 2: maxproc cannot be set outside of container: PASSED\n");
	} else {
		die("Level 2: maxproc cannot be set outside of container: FAILED");
	}

	// Test that maxproc has to be >0
	pid = fork();
	if (pid < 0) die("Level 2: fork: FAILED");
	else if (pid == 0) {
		// Enter a container
		if (cm_create_and_enter() == -1) die("Level 2: cm_create_and_enter: FAILED");

		result = cm_maxproc(0);
		if (result == -1) {
			printf(1, "Level 2: maxproc must be >0: PASSED\n");
		} else {
			die("Level 2: maxproc must be >0: FAILED");
		}

		exit();
	}
	else {
		wait();
	}

	// Test restricting the number of processes
	pid = fork();
	if (pid < 0) die("Level 2: fork: FAILED");
	else if (pid == 0) {
		// Enter a container
		if (cm_create_and_enter() == -1) die("Level 2: cm_create_and_enter: FAILED");

		// Test that maxproc can be set
		result = cm_maxproc(1);
		if (result == 0) {
			printf(1, "Level 2: maxproc = 1 set: PASSED\n");
		} else {
			die("Level 2: maxproc = 1 set: FAILED");
		}

		// Test that maxproc cannot be set twice
		result = cm_maxproc(2);
		if (result == -1) {
			printf(1, "Level 2: maxproc cannot be re-set: PASSED\n");
		} else {
			die("Level 2: maxproc cannot be re-set: FAILED");
		}

		// Test that maxproc restricts the number of processes (since we set it to 1, we should not be able to fork)
		int pid2 = fork();
		if (pid2 == -1) {
			printf(1, "Level 2: maxproc = 1 restricts the number of processes: PASSED\n");
		}
		else if (pid2 == 0) {
			// In child process, should not be able to fork so exit
			die("Level 2: maxproc = 1 restricts the number of processes: FAILED");
		}
		else {
			wait();
		}

		exit();
	}
	else {
		wait();
	}

	pid = fork();
	if (pid < 0) die("Level 2: fork: FAILED");
	else if (pid == 0) {
		// Enter a container
		if (cm_create_and_enter() == -1) die("Level 2: cm_create_and_enter: FAILED");

		if (cm_maxproc(2) == -1) die("Level 2: cm_maxproc: FAILED");

		// Test that maxproc restricts the number of processes (since we set it to 2, we should be able to fork once)
		int pid2 = fork();
		if (pid2 < 0) die("Level 2: fork: FAILED");
		else if (pid2 == 0) {
			// In child process, should be able to fork, so try to fork again
			int pid3 = fork();
			if (pid3 == -1) {
				printf(1, "Level 2: maxproc = 2 restricts the number of processes: PASSED\n");
			}
			else if (pid3 == 0) {
				// In child process, should not be able to fork so exit
				die("Level 2: maxproc = 2 restricts the number of processes: FAILED");
			}
			else {
				wait();
			}

			exit();
		}
		else {
			wait();
		}
		exit();
	} else {
		wait();
	}
}

void
level3(void)
{
	int pid;

	// Try going into a container and running container-test
	printf(1, "Level 3: creating container with c0-0.json. ../private/container.txt exists.\n\t");
	pid = fork();
	if (pid < 0) die("Level 3: fork: FAILED");
	else if (pid == 0) {
		cm("c0-0.json");

		exit();
	} else {
		wait();
	}

	printf(1, "Level 3: creating container with c1-0.json ../private/container.txt does not exist.\n\t");
	pid = fork();
	if (pid < 0) die("Level 3: fork: FAILED");
	else if (pid == 0) {
		cm("c1-0.json");

		exit();
	} else {
		wait();
	}
}
int
main(void)
{
	// Check if `testsetup` was already run, if not, run it
	int fd = open("c0", O_RDONLY);
	if (fd == -1) die("Test environment not properly set up. Run `setup` before running these tests");

	printf(1, "Starting to test Module 1: Container Manager & Kernel Container Support\n");
	printf(1, "-------------------------------------\n");


	// Level 0: Implement the logic to send to the CM the container specification. Parse and print out the configuration correctly.
	level0();

	printf(1, "-------------------------------------\n");

	// Level 1: Implement and test the system call to change the root in the fs of a process. You can test this
	// outside of the context of the CM. Make sure to thoroughly test this including proper handling of “..”, and
	// use of the system call within a process tree that is already working in a restricted root environment. Switch
	// to c0 folder
	level1();

	printf(1, "-------------------------------------\n");

	// Level 2: Level 2: Implement and test the system call for setting the maximum number of processes for a subtree of the process hierarchy. Test this completely.
	level2();

	printf(1, "-------------------------------------\n");

	// Level 3: Integrate the use of these two system calls into the CM along with the container specification to implement the resource/namespace constraining functionality of the CM. This should run the container’s init program.
	level3();

	printf(1, "-------------------------------------\n");

	exit();
	return 0;
}