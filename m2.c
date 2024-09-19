#include "types.h"
#include "user.h"
#include "stat.h"
#include "param.h"

void
createChildLvl1(void)
{
	int pid = fork();

	// If fork failed
	if (pid < 0) exit();

	// If in the child
	if (pid == 0) {
		char *test2 = shm_get("Level1_ForkedTest");
		if (test2) {
			printf(1, "Level 1: shm_get allocates shared memory w/two procs: PASSED\n");
			void *child_retaddr  = shm_retaddr("Level1_ForkedTest");
			void *parent_retaddr = shm_retaddr("Lvl1_Test");
			if (child_retaddr == (void *)2147471360 && parent_retaddr == (void *)2147475456) {
				printf(1, "Level 1: returns the correct address parent and child: PASSED\n");
			} else {
				printf(1, "Level 1: returns the correct address parent and child: FAILED\n");
			}
			exit();
		} else {
			printf(1, "Level 1: shm_get allocates shared memory w/two procs: FAILED\n");
			exit();
		}
	} else {
		wait();
	}
}

void
createChildLvl2(void)
{
	int pid = fork();
	//  If fork failed
	if (pid < 0) exit();

	// If in the child
	if (pid == 0) {
		char *test2 = shm_get("Test_Lvl2_Fork");
		if (test2) {
			printf(1, "Level 2: shm_get allocates shared memory between two procs: PASSED\n");
			void *test1_retaddr = shm_retaddr("Test_Lvl2");
			if (test1_retaddr == (void *)2147471360) {
				printf(1, "Level 2: returns the correct address: PASSED\n");
			} else {
				printf(1, "Level 2: returns the correct address: FAILED\n");
			}
			exit();
		} else {
			printf(1, "Level 2: shm_get allocates shared memory between two procs: FAILED\n");
			exit();
		}
	} else {
		wait();
	}
}


void
level0(void)
{
	printf(1, "\n");
	printf(1, "Testing Level 0\n");
	printf(1, "-------------------------------------\n");

	char *test = shm_get("Lvl0_Test");
	if (test) {
		printf(1, "Level 0: shm_get compiles and runs: PASSED\n");
	} else {
		printf(1, "Level 0: shm_get compiles and runs: FAILED\n");
	}

	void *test1_retaddr = shm_retaddr("Lvl0_Test");
	if (test1_retaddr == (void *)2147479552) {
		printf(1, "Level 0: returns the correct address parent: PASSED\n");
	} else {
		printf(1, "Level 0: returns the correct address parent: FAILED\n");
	}

	// Test calling get twice with same name
	char *test2 = shm_get("Lvl0_Test");
	if (test2) {
		printf(1, "Level 0: subsequent calls to shm_get: PASSED\n");
	} else {
		printf(1, "Level 0: subsequent calls to shm_get: FAILED\n");
	}

	void *test2_retaddr = shm_retaddr("Lvl0_Test");
	if (test2_retaddr == (void *)2147479552) {
		printf(1, "Level 0: returns the correct address: PASSED\n");
	} else {
		printf(1, "Level 0: returns the correct address: FAILED\n");
	}


	int rem_test = shm_rem("Lvl0_Test");
	if (rem_test != -1) {
		printf(1, "Level 0: shm_rem compiles and runs: PASSED\n");
	} else {
		printf(1, "Level 0: shm_rem compiles and runs: FAILED\n");
	}


	// Test calling get after removing and checking memory address
	char *test3 = shm_get("Level0_Test2");
	if (test3) {
		printf(1, "Level 0: shm_get compiles and runs after remove: PASSED\n");
	} else {
		printf(1, "Level 0: shm_get compiles and runs after remove: FAILED\n");
	}


	// Test calling remove twice
	int rem_test2 = shm_rem("Lvl0_Test");
	int rem_test3 = shm_rem("Lvl0_Test");

	if (rem_test2 != -1 && rem_test3 == -1) {
		printf(1, "Level 0: throws error if removing twice in same proc: PASSED\n");
	} else {
		printf(1, "Level 0: throws error if removing twice in same proc: FAILED\n");
	}


	void *test3_retaddr = shm_retaddr("Lvl0_Test");
	if (test3_retaddr == (void *)-1) {
		printf(1, "Level 0: returns the correct address: PASSED\n");
	} else {
		printf(1, "Level 0: returns the correct address: FAILED\n");
	}
}

void
level1(void)
{
	printf(1, "\n");
	printf(1, "TESTING LEVEL 1\n");
	printf(1, "-------------------------------------\n");


	char *test = shm_get("Lvl1_Test");
	if (test) {
		printf(1, "Level 1: shm_get one proc: PASSED\n");
	} else {
		printf(1, "Level 1: shm_get one proc: FAILED\n");
	}

	void *test1_retaddr = shm_retaddr("Lvl1_Test");
	if (test1_retaddr == (void *)2147475456) {
		printf(1, "Level 1: returns the correct address: PASSED\n");
	} else {
		printf(1, "Level 1: returns the correct address: FAILED\n");
	}

	createChildLvl1();

	int rem_test = shm_rem("Lvl1_Test");
	if (rem_test != -1) {
		printf(1, "Level 1: shm_rem deallocates shared memory: PASSED\n");
	} else {
		printf(1, "Level 1: shm_rem deallocates shared memory: FAILED\n");
	}

	void *test2_retaddr = shm_retaddr("Lvl1_Test");
	if (test2_retaddr == (void *)-1) {
		printf(1, "Level 1: returns the correct address: PASSED\n");
	} else {
		printf(1, "Level 1: returns the correct address: FAILED\n");
	}
}

void
level2(void)
{
	printf(1, "\n");
	printf(1, "TESTING LEVEL 2\n");
	printf(1, "-------------------------------------\n");


	char *test = shm_get("Test_Lvl2");
	if (test) {
		printf(1, "Level 2: shm_get allocates shared memory to one proc: PASSED\n");
	} else {
		printf(1, "Level 2: shm_get allocates shared memory to one proc: FAILED\n");
	}

	void *test1_retaddr = shm_retaddr("Test_Lvl2");
	if (test1_retaddr == (void *)2147471360) {
		printf(1, "Level 2: returns the correct address: PASSED\n");
	} else {
		printf(1, "Level 2: returns the correct address: FAILED\n");
	}

	createChildLvl2();

	int rem_test = shm_rem("Test_Lvl2");
	if (rem_test != -1) {
		printf(1, "Level 2: shm_rem deallocates shared memory: PASSED\n");
	} else {
		printf(1, "Level 2: shm_rem deallocates shared memory: FAILED\n");
	}

	void *test2_retaddr = shm_retaddr("Test_Lvl2");
	if (test2_retaddr == (void *)-1) {
		printf(1, "Level 2: returns the correct address: PASSED\n");
	} else {
		printf(1, "Level 2: returns the correct address: FAILED\n");
	}
}

void
level3(void)
{
	printf(1, "\n");
	printf(1, "TESTING LEVEL 3\n");
	printf(1, "-------------------------------------\n");

	char *SHM_1 = shm_get("SHM_1");
	char *SHM_2 = shm_get("SHM_2");
	char *SHM_3 = shm_get("SHM_3");
	char *SHM_4 = shm_get("SHM_4");

	void * test1_retaddr = shm_retaddr("SHM_1");
	void * test2_retaddr = shm_retaddr("SHM_2");
	void * test3_retaddr = shm_retaddr("SHM_3");
	void * test4_retaddr = shm_retaddr("SHM_4");


	if (SHM_1 && SHM_2 && SHM_3 && SHM_4) {
		printf(1, "Level 3: shm_get allocates 4 different shared memory pages for a single process: PASSED\n");
		if(test1_retaddr == (void *)2147467264 && test2_retaddr == (void *)2147463168 && test3_retaddr == (void *)2147459072 && test4_retaddr == (void *)2147454976){
			printf(1, "Level 3: returns the correct address: PASSED\n");
		}
		else{
			printf(1, "Level 3: returns the correct address: FAILED\n");
		}
	} else {
		printf(1, "Level 3: shm_get allocates 4 different shared memory pages for a single process: FAILED\n");
	}

	int rem_test = shm_rem("SHM_2");
	if (rem_test != -1) {
		printf(1, "Level 3: shm_rem deallocates shared memory: PASSED\n");
		void * test2_retaddr = shm_retaddr("SHM_2");
		if(test2_retaddr == (void *)-1){
			printf(1, "Level 3: returns the correct address: PASSED\n");
		}
		else{
			printf(1, "Level 3: returns the correct address: FAILED\n");
		}
	} else {
		printf(1, "Level 3: shm_rem deallocates shared memory: FAILED\n");
	}

	int pid = fork();
	if (pid < 0) exit();
	if (pid == 0) {
		// In the child
		char *SHM_5 = shm_get("SHM_5");
		char *SHM_6 = shm_get("SHM_6");
		if (SHM_5 && SHM_1 && SHM_2 && SHM_3 && SHM_4 && SHM_6) {
			printf(1, "Level 3: shm_get allocates shared memory to a child process: PASSED\n");
			printf(1, "Level 3: fork() copies over existing shared memory: PASSED\n");
			void * shm_5ret = shm_retaddr("SHM_5");
			void * shm_6ret = shm_retaddr("SHM_6");
			if(shm_5ret == (void *)2147454976 && shm_6ret == (void *)2147450880){
				printf(1, "Level 3: returns the correct address: PASSED\n");
			}
			else{
				printf(1, "Level 3: returns the correct address: FAILED\n");
			}

			int pid2 = fork();
			if (pid2 < 0) exit();
			if(pid2 == 0)
			{
				//In the grandchild
				char *SHM_7 = shm_get("SHM_7");
				char *SHM_8 = shm_get("SHM_8");
				if(SHM_7 && SHM_8 && SHM_1 && SHM_2 && SHM_3 && SHM_4 && SHM_5 && SHM_6){
					printf(1, "Level 3: shm_get allocates shared memory to a grandchild process: PASSED\n");
					printf(1, "Level 3: fork() copies over existing shared memory: PASSED\n");
					void * shm_7ret = shm_retaddr("SHM_7");
					void * shm_8ret = shm_retaddr("SHM_8");
					if(shm_7ret == (void *)2147446784 && shm_8ret == (void *)2147442688){
						printf(1, "Level 3: returns the correct address: PASSED\n");
					}
					else{
						printf(1, "Level 3: returns the correct address: FAILED\n");
					}
				}
				else{
					printf(1, "Level 3: shm_get allocates shared memory to a grandchild process: FAILED\n");
					printf(1, "Level 3: fork() copies over existing shared memory: FAILED\n");
				}
			}

		} else {
			printf(1, "Level 3: shm_get allocates shared memory to a child process: FAILED\n");
			printf(1, "Level 3: fork() copies over existing shared memory: FAILED\n");
		}
	}

	wait();
	exit();
}


int
main(void)
{
	level0();
	level1();
	level2();
	level3();
	exit();
}
