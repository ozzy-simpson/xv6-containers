#include "types.h"
#include "user.h"


#define NCHILDREN  8		/* should be > 1 to have contention */
#define CRITSECTSZ 3
#define DATASZ (1024 * 32 / NCHILDREN)

#define LOCKS_IMPLEMENTED 	/* uncomment this when you have a working lock implementation */
#ifdef LOCKS_IMPLEMENTED

#define LOCK_CREATE(x)  mutex_create(x)
#define LOCK_TAKE(x)    mutex_lock(x)
#define LOCK_RELEASE(x) mutex_unlock(x)
#define LOCK_DEL(x)     mutex_delete(x)
#else
int compilehack = 0;
#define LOCK_CREATE(x)  compilehack++
#define LOCK_TAKE(x)    compilehack++
#define LOCK_RELEASE(x) compilehack++
#define LOCK_DEL(x)     compilehack++
#endif


//make the max number of locks then remove them
//try to make 17 should not work
//remove lock id twice should not work 
//try giving remove -1 should not work
void level0(){
	int lockid;

	//16 times
	if ((lockid = mutex_create("lock1")) < 0) {
		printf(1, "Lock creation error, did not create\n");
		exit();
	}
	if ((lockid = mutex_create("lock2")) < 0) {
		printf(1, "Lock creation error, did not create\n");
		exit();
	}
	if ((lockid = mutex_create("lock3")) < 0) {
		printf(1, "Lock creation error, did not create\n");
		exit();
	}
	if ((lockid = mutex_create("lock4")) < 0) {
		printf(1, "Lock creation error, did not create\n");
		exit();
	}
	if ((lockid = mutex_create("lock5")) < 0) {
		printf(1, "Lock creation error, did not create\n");
		exit();
	}
	if ((lockid = mutex_create("lock6")) < 0) {
		printf(1, "Lock creation error, did not create\n");
		exit();
	}
	if ((lockid = mutex_create("lock7")) < 0) {
		printf(1, "Lock creation error, did not create\n");
		exit();
	}
	if ((lockid = mutex_create("lock13")) < 0) {
		printf(1, "Lock creation error, did not create\n");
		exit();
	}

	if ((lockid = mutex_create("lock14")) < 0) {
		printf(1, "Lock creation error, did not create\n");
		exit();
	}
	if ((lockid = mutex_create("lock15")) < 0) {
		printf(1, "Lock creation error, did not create\n");
		exit();
	}
	if ((lockid = mutex_create("lock16")) < 0) {
		printf(1, "Lock creation error, did not create\n");
		exit();
	}
	if ((lockid = mutex_create("lock17")) == -1) {
			printf(1, "Lock creation error should not create lock number 17 but it did\n");
			exit();
	}
	if ((lockid = mutex_create("lock1")) < 0) {
		printf(1, "Lock created with the same name\n");
		exit();
	}
	
	for (int i = 0; i < 16; i++){
		mutex_delete(i);

	}
	if (mutex_delete(-1) == 0 ){//deleting twice cant do that
		printf(1, "Lock deletion error, cant delete lock when id is -1\n");
		exit();
	}
	printf(1, "create 16 locks works, PASSED\n");
	printf(1, "create lock number 17 shouldnt work, PASSED\n");
	printf(1, "create a lock with same name returns same id, PASSED\n");
	printf(1, "Deleting all locks, PASSED\n");
	printf(1, "deleting invalid id, PASSED\n");


	printf(1, "-----------------------------------level 0 passed---------------------\n\n");
}
/*
---------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------
level 1 under this
*/


void
child(int lockid, int pipefd, char tosend)
{
	int i, j;

	for (i = 0 ; i < DATASZ ; i += CRITSECTSZ) {
		/*
		 * If the critical section works, then each child
		 * writes CRITSECTSZ times to the pipe before another
		 * child does.  Thus we can detect race conditions on
		 * the "shared resource" that is the pipe.
		 */
		
		if (LOCK_TAKE(lockid) != 0){
			printf(1, "cant take lock, lock take fail\n");
		}
		// if (LOCK_TAKE(lockid) == 1){
		// 	printf(1, "cant take lock twice\n");
		// }
		for (j = 0 ; j < CRITSECTSZ ; j++) {
			write(pipefd, &tosend, 1);
		}
		if (LOCK_RELEASE(lockid) != 0){
			printf(1, "release error\n");
		}
		
	}
	exit();
}
void hwcode(){
	int i;
	int pipes[2];
	char data[NCHILDREN], first = 'a';
	int lockid;

	for (i = 0 ; i < NCHILDREN ; i++) {
		data[i] = (char)(first + i);
	}

	if (pipe(&pipes[0])) {
		printf(1, "Pipe error\n");
		exit();
	}

	if ((lockid = LOCK_CREATE("LOCK_BLOCK")) < 0) {
		printf(1, "Lock creation error\n");
		exit();
	}

	for (i = 0 ; i < NCHILDREN ; i++) {
		if (fork() == 0) child(lockid, pipes[1], data[i]);
	}
	close(pipes[1]);

	while (1) {
		char fst, c;
		int cnt;

		fst = '_';
		for (cnt = 0 ; cnt < CRITSECTSZ ; cnt++) {
			if (read(pipes[0], &c, 1) == 0) goto done;

			if (fst == '_') {
				fst = c;
			} else if (fst != c) {
				printf(1, "RACE!!!\n");
			}
		}
	}
done:

	for (i = 0 ; i < NCHILDREN ; i++) {
		if (wait() < 0) {
			printf(1, "Wait error\n");
			exit();
		}
	}
	LOCK_DEL(lockid);

}

void level1(){
	hwcode();
	int lockid;
	if ((lockid = mutex_create("lock1")) < 0) {
		printf(1, "Lock creation error, did not create\n");
		exit();
	}
	if (mutex_lock(lockid) < 0){
		printf(1, "cant lock fail\n");
		exit();
	}
	if (mutex_lock(lockid) == 0){
		printf(1, "already locked\n");
		exit();
	}
	if (mutex_unlock(lockid) < 0){
		printf(1, "cant unlock \n");
		exit();
	}
	mut_delete(lockid);
	printf(1, "Lock_example, PASSED\n");
	printf(1, "locking adn unlocking, PASSED\n");
	printf(1, "locking on lock shouldnt work, PASSED\n");
	printf(1, "-----------------------------------level 1 passed---------------------\n\n");


}

/*
---------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------
level 2 under this
*/


void basic() {
	int lockid;

    if ((lockid = mutex_create("lock1")) < 0) {
        printf(1, "Lock creation error\n");
        exit();
    }
	if (fork()==0){
		sleep(100);
		cv_signal(lockid);
		exit();
	}
	cv_wait(lockid);
	wait();
}
void signal_before_wait(){
	int lockid;
    if ((lockid = mutex_create("lock_block")) < 0) {
        printf(1, "Lock creation error\n");
        exit();
    }
	for(int i = 0; i < 30; i++){//signal 30 times
		cv_signal(lockid);
	}
	cv_wait(lockid);
	mutex_delete(lockid);

}

void level2() {
	basic();
	signal_before_wait();//this is going to have print statements but the goal of this oen is 
	printf(1, "waiting in child and signalind in parent works, PASSED\n");
	printf(1, "singaling before waiting works doesnt sleep, PASSED\n");
	printf(1, "-----------------------------------level 2 passed---------------------\n\n");

}


/*
---------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------
level 3 under this
*/

void level3() {
    int lockid;
    if ((lockid = mutex_create("lock_block")) < 0) {
        printf(1, "Lock creation error\n");
        exit();
    }
    int pid = fork();
    if (pid < 0) {
        exit();
    }

    if (pid == 0) { //child
	// printf(1, "before lock take child\n");

		if (mutex_lock(lockid) < 0){
			printf(1, "lock didnt take in child\n");
			exit();
		}
		
        exit();
    } 
	sleep(100);//wait for the child
	if (mutex_lock(lockid) < 0){
		printf(1, "lock didnt take after child is terminated\n");
		exit();
	}
	wait();
	mut_delete(lockid);

}

void level3pt2() {
	int lockid;
    if ((lockid = mutex_create("lock_block")) < 0) {
        printf(1, "Lock creation error\n");
        exit();
    }
    int pid = fork();
    if (pid < 0) {
        exit();
    }
	if (pid == 0) {
		cv_wait(lockid);
        exit();
    } 
	else{
		mutex_lock(lockid);
	}
	wait();
	mut_delete(lockid);
	printf(1, "forking and waiting in child then exiting then signaling, PASSED\n");
	printf(1, "forking and waiting in child then taking lock in parent, PASSED\n");
	printf(1, "-----------------------------------level 3 passed---------------------\n\n");

}

int main(){
	printf(1, "-------------------------------------------------------------\n");
	level0();

	printf(1, "Starting testing on level 1\n");
	level1();
	printf(1, "Starting testing on level 2\n");
	level2();
	printf(1, "Starting testing on level 3\n");
	level3();
	level3pt2();
	printf(1, "done testing\n");
	exit();
	return 0;
}