# Container management and communication in xv6

This was a group project in GW's Operating Systems course. The goal was to implement container management and communication in the xv6 UNIX operating system. The project was broken down into four modules, each with four levels of increasing difficulty. I focused on the container manager module, which involved setting up containers and restricting processes to those containers. The other modules involved shared memory, synchronization, and scheduling.

## Module #1: Container Manager

Levels 0–3 are implemented, as described below:

- [x] Level 0: Implement the logic to send to the CM the container specification. Parse and print out the configuration correctly.

- [x] Level 1: Implement and test the system call to change the root in the fs of a process. You can test this outside of the context of the CM. Make sure to thoroughly test this including proper handling of “..”, and use of the system call within a process tree that is already working in a restricted root environment.

- [x] Level 2: Implement and test the system call for setting the maximum number of processes for a subtree of the process hierarchy. Test this completely.
- [x] Level 3: Integrate the use of these two system calls into the CM along with the container specification to implement the resource/namespace constraining functionality of the CM. This should run the container’s init program.
- [ ] Level 4: Integrate the stdin/stdout/stderr routing through the CM from the container to the client and vice-versa. This will likely require using multiple processes so that different processes can block waiting for input from outside the container to send to the stdin within the container, and to block waiting for input from the stdout of the container to output onto the terminal. This should be possible with multiple containers created by the same CM. This level is hard. Please see the “Overall Project” guidelines.
Level Testing Documentation

### Testing

In general, running `setup` and then [`m1`](m1.c) will run the automated tests for the container manager.

[`setup`](setup.c) runs the [`testsetup()`](testsetup.c) function that creates folders for multiple containers, copies user-level programs into the containers, creates a `container.txt` file with the container's number in it, and creates a `private` sub-folder in one container with the `container.txt` file in it as well.

#### Level 1

[Level 1](m1.c#L31) is tested by setting the root of a forked process to a container and then ensuring the `container.txt` file is at the root with the correct container number in it.

Other tests for this level test the edge cases of the `set_container_root()` function, including: setting the root twice; setting the root to a file (not a directory); setting the root to a non-existent directory; trying to `chdir()` and open files out of the set root; as well as a multitude of tests with various `.` and `..` in the path; the root is inherited by and enforced in a child (and cannot be changed by the child). It also tests running user-level programs in the container with the root set, to ensure that those programs are also restricted to the container.

#### Level 2

[Level 2](m1.c#L574) is testing by ensuring that `maxproc` can only be set within a container (tested inside and outside of a container), that it must be >0, that it can be set only once, and that the number of processes in a container is limited by `maxproc` (tested by forking, with various values of `maxproc`).

#### Level 3

[Level 3](m1.c#L683) is tested by calling the container manager on two different container specs ([c0-0.json](c0-0.json) and [c1-0.json](c1-0.json)). Within those containers, it runs a [`containertest`](containertest.c) user-level program that:

- prints the container number
- attempts to fork bomb the container, to test how many processes can be created in the container
- tries to access a file within a sub-folder that only exists in one of the containers (which should fail in the other container). This tries to access the sub-folder by going outside of the root, to test that the root is also enforced in the container

#### Level 4

Level 4 is not implemented with this module, so it is not tested.
