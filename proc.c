#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "containers.h"
#include "stat.h"
#include "fs.h"
#include "file.h"

struct {
	struct spinlock lock;
	struct proc     proc[NPROC];
} ptable;

struct mutex{
	char* name;
	int id;//might delete
	int exists;// if exists in global table
	uint taken; //0 for not taken, 1 for taken
	struct spinlock lock;
	struct sleeplock slock;
	int numProcs; //number of process that have taken that lock
	int cid; //container id
};

struct {
	struct spinlock lock;
	struct mutex mux_table[16];
} mtable;

struct pq1_list {
	struct proc *head;
	struct proc *tail;
	int         length;
};

struct {
	struct pq1_list lists[PRIO_MAX + 1 - PRIO_LOW];
} fp;

static struct proc *initproc;

int         nextpid = 1;
int         nextcid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
	initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid()
{
	return mycpu() - cpus;
}

// Must be called with interrupts disabled to avoid the caller being rescheduled
// between reading lapicid and running through the loop.
struct cpu *
mycpu(void)
{
	int apicid, i;

	if (readeflags() & FL_IF) panic("mycpu called with interrupts enabled\n");

	apicid = lapicid();
	// APIC IDs are not guaranteed to be contiguous. Maybe we should have
	// a reverse map, or reserve a register to store &cpus[i].
	for (i = 0; i < ncpu; ++i) {
		if (cpus[i].apicid == apicid) return &cpus[i];
	}
	panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc *
myproc(void)
{
	struct cpu * c;
	struct proc *p;
	pushcli();
	c = mycpu();
	p = c->proc;
	popcli();
	return p;
}

// PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc *
allocproc(void)
{
	struct proc *p;
	char *       sp;

	acquire(&ptable.lock);

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
		if (p->state == UNUSED) goto found;

	release(&ptable.lock);
	return 0;

found:
	p->state = EMBRYO;
	p->pid   = nextpid++;
	p->priority = 0; // default priority is 0
	p->cid      = 0; // default cid is 0
	p->maxproc  = 0; // default maxproc is 0
	p->root     = 0; // default root is 0

	release(&ptable.lock);

	// Allocate kernel stack.
	if ((p->kstack = kalloc()) == 0) {
		p->state = UNUSED;
		return 0;
	}
	sp = p->kstack + KSTACKSIZE;

	// Leave room for trap frame.
	sp -= sizeof *p->tf;
	p->tf = (struct trapframe *)sp;

	// Set up new context to start executing at forkret,
	// which returns to trapret.
	sp -= 4;
	*(uint *)sp = (uint)trapret;

	sp -= sizeof *p->context;
	p->context = (struct context *)sp;
	memset(p->context, 0, sizeof *p->context);
	p->context->eip = (uint)forkret;

	return p;
}

// PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
	struct proc *p;
	extern char  _binary_initcode_start[], _binary_initcode_size[];

	p = allocproc();


	initproc = p;
	if ((p->pgdir = setupkvm()) == 0) panic("userinit: out of memory?");
	inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
	p->sz = PGSIZE;
	memset(p->tf, 0, sizeof(*p->tf));
	p->tf->cs     = (SEG_UCODE << 3) | DPL_USER;
	p->tf->ds     = (SEG_UDATA << 3) | DPL_USER;
	p->tf->es     = p->tf->ds;
	p->tf->ss     = p->tf->ds;
	p->tf->eflags = FL_IF;
	p->tf->esp    = PGSIZE;
	p->tf->eip    = 0; // beginning of initcode.S

	safestrcpy(p->name, "initcode", sizeof(p->name));
	p->cwd = namei("/");

	// this assignment to p->state lets other cores
	// run this process. the acquire forces the above
	// writes to be visible, and the lock is also needed
	// because the assignment might not be atomic.
	acquire(&ptable.lock);

	p->state = RUNNABLE;

	release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
	uint         sz;
	struct proc *curproc = myproc();

	sz = curproc->sz;
	if (n > 0) {
		if ((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0) return -1;
	} else if (n < 0) {
		if ((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0) return -1;
	}
	curproc->sz = sz;
	switchuvm(curproc);
	return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
	int          i, pid;
	struct proc *np;
	struct proc *curproc = myproc();

	// Check that we haven't exceeded maxproc, if set and in a container
	if (curproc->maxproc != 0 && curproc->cid != 0) {
		int nproc = 0;
		struct proc *p;

		// Count number of processes in container
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
			if (p->cid == curproc->cid) nproc++;
		}

		// If we have exceeded maxproc, return error
		if (nproc >= curproc->maxproc) return -1;
	}

	// Allocate process.
	if ((np = allocproc()) == 0) {
		return -1;
	}

	// Copy process state from proc.
	if ((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0) {
		kfree(np->kstack);
		np->kstack = 0;
		np->state  = UNUSED;
		return -1;
	}
	np->sz     = curproc->sz;
	np->parent = curproc;
	np->priority = curproc->priority; // copy priority from parent
	*np->tf    = *curproc->tf;

	// Container info
	np->cid     = curproc->cid; // copy cid from parent
	np->maxproc = curproc->maxproc; // copy maxproc from parent
	np->root    = curproc->root; // copy root from parent

	// Clear %eax so that fork returns 0 in the child.
	np->tf->eax = 0;

	//increment proc count when i fork
	
	for (i = 0; i < NOFILE; i++)
		if (curproc->ofile[i]) np->ofile[i] = filedup(curproc->ofile[i]);
	np->cwd = idup(curproc->cwd);

	safestrcpy(np->name, curproc->name, sizeof(curproc->name));

	pid = np->pid;

	//Copy the map of shared pages to the new process 
	memmove(np->shm_addr, curproc->shm_addr, sizeof(curproc->shm_addr));

	//Add in shared memory new proc for the pages of the current proc 
	if(shm_forkcpy(curproc, np) < 0){
		return -1;
	}
	acquire(&mtable.lock);

	for (i = 0; i < 16; i++){
		if(curproc->Mutexes[i]){
			if(curproc->Mutexes[i]->numProcs < 0){
				continue;
			}
			curproc->Mutexes[i]->numProcs++;
			np->Mutexes[i] = curproc->Mutexes[i];
		}
		
	}
	release(&mtable.lock);

	acquire(&ptable.lock);

	np->state = RUNNABLE;

	release(&ptable.lock);

	return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
	struct proc *curproc = myproc();
	struct proc *p;
	int          fd;

	if (curproc == initproc) panic("init exiting");

	//Remove from shared memory the pages that were referenced by the current proc
	shm_exit();
	//module 3 edits for level 3
	for(int i = 0; i < 16; i++){
		if(curproc->Mutexes[i]->taken == curproc->pid){
			mut_delete(curproc->Mutexes[i]->id);
		}
	}
	curproc->numMutex=0;
	// Close all open files.
	for (fd = 0; fd < NOFILE; fd++) {
		if (curproc->ofile[fd]) {
			fileclose(curproc->ofile[fd]);
			curproc->ofile[fd] = 0;
		}
	}

	begin_op();
	iput(curproc->cwd);
	end_op();
	curproc->cwd = 0;

	// Reset container info
	curproc->cid     = 0;
	curproc->maxproc = 0;
	curproc->root    = 0;

	acquire(&ptable.lock);

	// Parent might be sleeping in wait().
	wakeup1(curproc->parent);

	// Pass abandoned children to init.
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->parent == curproc) {
			p->parent = initproc;
			if (p->state == ZOMBIE) wakeup1(initproc);
		}
	}

	// Jump into the scheduler, never to return.
	curproc->state = ZOMBIE;
	sched();
	panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
	struct proc *p;
	int          havekids, pid;
	struct proc *curproc = myproc();

	acquire(&ptable.lock);
	for (;;) {
		// Scan through table looking for exited children.
		havekids = 0;
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
			if (p->parent != curproc) continue;
			havekids = 1;
			if (p->state == ZOMBIE) {
				// Found one.
				pid = p->pid;
				kfree(p->kstack);
				p->kstack = 0;
				freevm(p->pgdir);
				p->pid     = 0;
				p->parent  = 0;
				p->name[0] = 0;
				p->killed  = 0;
				p->state   = UNUSED;
				release(&ptable.lock);
				return pid;
			}
		}

		// No point waiting if we don't have any children.
		if (!havekids || curproc->killed) {
			release(&ptable.lock);
			return -1;
		}

		// Wait for children to exit.  (See wakeup1 call in proc_exit.)
		sleep(curproc, &ptable.lock); // DOC: wait-sleep
	}
}

// PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
	struct proc *p;
	struct pq1_list *list;
	struct cpu * c = mycpu();
	c->proc        = 0;

	for (;;) {
		// Enable interrupts on this processor.
		sti();

		// Loop over process table looking for process to run.
		acquire(&ptable.lock);
		for (list = &fp.lists[PRIO_LOW]; list <= &fp.lists[PRIO_MAX]; list++) {
			if (list->length == 0) continue; // skip empty lists

			// Get first process in list
			p = list->head;
			
			// Find first runnable process in list
			while (p != 0 && p->state != RUNNABLE) {
				p = p->next;
			}

			// If no runnable process in list, continue to next list
			if (p == 0) continue;

			// Switch to chosen process.  It is the process's job
			// to release ptable.lock and then reacquire it
			// before jumping back to us.
			c->proc = p;
		
			switchuvm(p);
			p->state = RUNNING;

			swtch(&(c->scheduler), p->context);
			switchkvm();

			// Process is done running for now.
			// It should have changed its p->state before coming back.
			c->proc = 0;
		}
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
			if (p->state != RUNNABLE) continue;

			// Switch to chosen process.  It is the process's job
			// to release ptable.lock and then reacquire it
			// before jumping back to us.
			c->proc = p;
			switchuvm(p);
			p->state = RUNNING;

			swtch(&(c->scheduler), p->context);
			switchkvm();

			// Process is done running for now.
			// It should have changed its p->state before coming back.
			c->proc = 0;
		}
		release(&ptable.lock);
	}
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should	
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
	int          intena;
	struct proc *p = myproc();

	if (!holding(&ptable.lock)) panic("sched ptable.lock");
	if (mycpu()->ncli != 1) panic("sched locks");
	if (p->state == RUNNING) panic("sched running");
	if (readeflags() & FL_IF) panic("sched interruptible");
	intena = mycpu()->intena;
	swtch(&p->context, mycpu()->scheduler);
	mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
	acquire(&ptable.lock); // DOC: yieldlock
	myproc()->state = RUNNABLE;
	sched();
	release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
	static int first = 1;
	// Still holding ptable.lock from scheduler.
	release(&ptable.lock);

	if (first) {
		// Some initialization functions must be run in the context
		// of a regular process (e.g., they call sleep), and thus cannot
		// be run from main().
		first = 0;
		iinit(ROOTDEV);
		initlog(ROOTDEV);
	}

	// Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
	struct proc *p = myproc();

	if (p == 0) panic("sleep");

	if (lk == 0) panic("sleep without lk");

	// Must acquire ptable.lock in order to
	// change p->state and then call sched.
	// Once we hold ptable.lock, we can be
	// guaranteed that we won't miss any wakeup
	// (wakeup runs with ptable.lock locked),
	// so it's okay to release lk.
	if (lk != &ptable.lock) {      // DOC: sleeplock0
		acquire(&ptable.lock); // DOC: sleeplock1
		release(lk);
	}
	// Go to sleep.
	p->chan  = chan;
	p->state = SLEEPING;

	sched();

	// Tidy up.
	p->chan = 0;

	// Reacquire original lock.
	if (lk != &ptable.lock) { // DOC: sleeplock2
		release(&ptable.lock);
		acquire(lk);
	}
}

// PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
	struct proc *p;

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
		if (p->state == SLEEPING && p->chan == chan) p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
	acquire(&ptable.lock);
	wakeup1(chan);
	release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
	struct proc *p;

	acquire(&ptable.lock);
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->pid == pid) {
			p->killed = 1;
			// Wake process from sleep if necessary.
			if (p->state == SLEEPING) p->state = RUNNABLE;
			release(&ptable.lock);
			return 0;
		}
	}
	release(&ptable.lock);
	return -1;
}

// PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
	static char *states[] = {[UNUSED] "unused",   [EMBRYO] "embryo",  [SLEEPING] "sleep ",
	                         [RUNNABLE] "runble", [RUNNING] "run   ", [ZOMBIE] "zombie"};
	int          i;
	struct proc *p;
	char *       state;
	uint         pc[10];

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->state == UNUSED) continue;
		if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
			state = states[p->state];
		else
			state = "???";
		cprintf("%d %s %s", p->pid, state, p->name);
		if (p->state == SLEEPING) {
			getcallerpcs((uint *)p->context->ebp + 2, pc);
			for (i = 0; i < 10 && pc[i] != 0; i++) cprintf(" %p", pc[i]);
		}
		cprintf("\n");
	}
}

// This attempts to set the priority of the
// process identified by pid to priority. However, this will successfully set the priority
// and return success (0) only if the current process is in the ancestry of the process with
// process id pid, or if pid identifies the current process. In either of those cases, the
// current process cannot raise the priority (numerically lower numbers) higher than the
// current process's priority. In all other cases, return an error (-
int
prio_set(int pid, int priority)
{
	if (priority < 0) priority = 0; // priority cannot be negative
	if (priority > PRIO_MAX) priority = PRIO_MAX; // priority cannot be lower than PRIO_MAX

	int cur_prio = myproc()->priority;

	// If priority is numerically lower than current process's priority, return error
	if (priority < cur_prio) return -1;

	struct proc *toset;

	// If pid is current process, toset is current process
	if (pid == myproc()->pid) {
		toset = myproc();
	}
	else {
		// Need to check if current process is in ancestry of process with pid
		// First, find process with pid
		struct proc *p;
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
			if (p->pid == pid) goto found;
		}
		
		// pid not found (no process with pid)
		return -1;

found:
		// Save the process with pid in toset, since this is the proc where we will set priority, if we can
		toset = p;

		// Now, check if current process is in ancestry of process with pid
		struct proc *cur = myproc();
		while (p->parent != cur) {
			if (p->parent == 0) return -1; // pid is not a descendant of current process since we found the initial proc
			p = p->parent; // move up the tree
		}

		// If we get here, then current process is in ancestry of process with pid, so we can continue
	}
	if(toset->is_fp==1){// If priority is already set, remove from existing priority queue
		int oldPriority = toset->priority;

		// Get list for priority
		struct pq1_list *list = &fp.lists[oldPriority];

		// Get previous node
		struct proc *prev = 0;
		struct proc *cur  = list->head;
		while (cur != 0) {
			if (cur == toset) break;
			prev = cur;
			cur  = cur->next;
		}

		// If cur is not null, then we found the node and can remove it
		if (cur != 0) {
			// Check if prev is null
			if (prev == 0) {
				// prev is null, so cur is head
				list->head = cur->next;
			}
			else {
				// prev is not null, so cur is not head
				prev->next = cur->next;
			}

			// Check if cur is tail
			if (cur == list->tail) {
				// cur is tail, so prev is new tail
				list->tail = prev;
			}
			// Decrement length
			list->length--;
		}
	}

	// Add to FP 
	struct pq1_list *list = &fp.lists[priority];

	// Check if list is empty
	if (list->head == 0) {
		// List is empty, so next is first/last (only) node
		list->head = list->tail = toset;
	}
	else {
		// List is not empty, so add after tail
		list->tail->next = toset;
		list->tail = toset;
	}

	// Set next to 0, since it is now the last node
	toset->next = 0;

	// Increment length
	list->length++;

	// Set priority and is_fp flag
	toset->priority = priority;
	toset->is_fp=1;
	//cprintf("about to return from setprio\n");
	return 0;
}

int
cm_create_and_enter(void)
{
	struct proc *curproc = myproc();

	// Check that we're not already in a container
	if (curproc->cid != 0) return -1;

	// Set up container by setting cid
	curproc->cid = nextcid++;

	return 0;
}

int
cm_setroot(char *path, int path_len)
{
	struct proc *curproc = myproc();

	// If root is already set, return error
	if (curproc->root != 0) return -1;

	// Check if new root is valid and a directory
	begin_op();
	struct inode *ip = namei(path);
	if (ip == 0) {
		end_op();
		return -1; // Invalid path
	}

	ilock(ip);

	if (ip->type != T_DIR) {
		// Not a directory
		iunlockput(ip);
		end_op();
		return -1;
	}
	iunlock(ip);

	// Switch to new root
	iput(curproc->cwd);
	end_op();
	curproc->cwd = ip;

	// Set root
	curproc->root = ip;

	return 0;
}

int
cm_maxproc(int nproc)
{
	struct proc *curproc = myproc();

	// Check if nproc is valid
	if (nproc < 1) return -1;

	// Check if we're in a container
	if (curproc->cid == 0) return -1;

	// Check if maxproc is already set
	if (curproc->maxproc != 0) return -1;

	// Set maxproc
	curproc->maxproc = nproc;

	return 0;
}

int mut_create(char * name){
	acquire(&mtable.lock);

	//same name share the same id
	int i= 0;
	for (i = 0; i < 16;i++){
		if(strncmp(mtable.mux_table[i].name,name, strlen(name))== 0 && myproc()->cid == mtable.mux_table[i].cid){
			release(&mtable.lock);
			return i;
		}
	}
	for(i=0; i < 16; i++){
		if(mtable.mux_table[i].exists != 1){//means there is a 
			mtable.mux_table[i].name = name;
			mtable.mux_table[i].id = i;
			mtable.mux_table[i].cid = myproc()->cid;
			mtable.mux_table[i].exists=1;
			mtable.mux_table[i].taken = 0;
			myproc()->Mutexes[i] = &mtable.mux_table[i];//assigned it in myproc
			mtable.mux_table[i].numProcs++;
			release(&mtable.lock);
			return mtable.mux_table[i].id;
		}
	}
	//error checking, no space in mux_array
	if(i==16){
		release(&mtable.lock);
		return -1;
	}

	release(&mtable.lock);

	return i;
}
int mut_delete(int muxid){
	if(muxid < 0 || muxid >= 16){
		return -1;
	}
	struct mutex *m = &mtable.mux_table[muxid];
	//not same mutex
	if(mtable.mux_table[muxid].taken > 0){
		mut_unlock(muxid);
	}
	if (myproc()->Mutexes[muxid] != m){
		return -1;
	}
	if (myproc()->cid != m->cid){
		// Not same container
		return -1;
	}
	
	if(m->taken !=0){
		mut_unlock(muxid);
	}

	
	acquire(&mtable.lock);
	mtable.mux_table[muxid].numProcs--;
	if(mtable.mux_table[muxid].numProcs >0 ){
		release(&mtable.lock);

		return -1;
	}
	

	if(mtable.mux_table[muxid].numProcs == 0){
		mtable.mux_table[muxid].exists = 0;
		mtable.mux_table[muxid].taken = 0;
	}
	release(&mtable.lock);
	return 0;
}
int mut_lock(int muxid){
	//mut_lock
	if (muxid < 0 || muxid >= 16) {
        return -1; 
    }

    struct mutex *l = &mtable.mux_table[muxid];
	if (myproc()->cid != l->cid){
		// Not same container
		return -1;
	}
    if (myproc()->Mutexes[muxid]->id == -1){
		return -1;
	}
	
	if (myproc()->Mutexes[muxid]->taken != 0 && l->taken == myproc()->pid){
		return -1;
	}

	
	if (mtable.mux_table[muxid].taken > 0){
		mut_unlock(muxid);
	}
	mtable.mux_table[muxid].taken = myproc()->pid;
	acquiresleep(&mtable.mux_table[muxid].slock);

    return 0;  // Success
}
int mut_unlock(int muxid){
	//error checking
    if (muxid < 0 || muxid >= 16) {
        return -1; 
    }
	if (myproc()->Mutexes[muxid]->id == -1){
		return -1;//invalid has been deleted
	}
	struct mutex *l = &mtable.mux_table[muxid];
	if (myproc()->cid != l->cid){
		// Not same container
		return -1;
	}
	int current = myproc()->pid;

	mtable.mux_table[muxid].taken = 0;//set it to 0 since no one is using it anymore
	//check if the process id is the same as the lock id so i can release it
	if (current == mtable.mux_table[muxid].slock.pid){
		releasesleep(&mtable.mux_table[muxid].slock);
	}

    return 0;
}

/*
The condition variable API is vastly simplified compared
to the pthread version. We assume that each mutex can have a single condition
associated with it. This works well if we just want the CM waiting on the condition that a
client sends a request, and that clients will block waiting on the condition that the CM
replies to them. Because of this, we need only specify the mutex id that the condition
variable is associated with, and not which condition variable to use.
*/
int c_wait(int muxid){
	//error checking
	if (muxid <0 || muxid >=16){
		return -1;
	}

	struct mutex *m = &mtable.mux_table[muxid];
	if (myproc()->cid != m->cid){
		// Not same container
		return -1;
	}
	struct proc *curproc = myproc();
	//no need to sleep since already signaled more than wait.
	if(curproc->signal_count > curproc->wait_count){
		curproc->signal_count--;
		return 0;
	}
	//unlock the mutex
	mut_unlock(muxid);
	
	acquire(&m->lock);

	curproc->wait_count++;
	curproc->sleeping = 1;
	
	//wait for it
	sleep(&m->lock, &m->lock);//twice

	release(&m->lock);

	//lock again
	mut_lock(muxid);
	return 0;
}

/*
wake up any threads that are blocked on the condition
variable associated with the mutex identified by its mutex id. This doesn’t alter the
mutex at all, and only wakes up any blocked threads. You can implement this such that
it wakes all threads, or just one.
*/
int c_signal(int muxid){
	//error checking
	if (muxid <0 || muxid >=16){
		return -1;
	}
	struct mutex *m = &mtable.mux_table[muxid];
	if (myproc()->cid != m->cid){
		// Not same container
		return -1;
	}
	struct proc *curproc = myproc();
	//increase singal count 
	curproc->signal_count++;
	acquire(&m->lock);
	wakeup1(&m->lock);//wakeup1 process
	release(&m->lock);

	
	return 0;
}