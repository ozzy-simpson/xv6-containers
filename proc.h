// Per-CPU state
struct cpu {
	uchar            apicid;     // Local APIC ID
	struct context  *scheduler;  // swtch() here to enter scheduler
	struct taskstate ts;         // Used by x86 to find stack for interrupt
	struct segdesc   gdt[NSEGS]; // x86 global descriptor table
	volatile uint    started;    // Has the CPU started?
	int              ncli;       // Depth of pushcli nesting.
	int              intena;     // Were interrupts enabled before pushcli?
	struct proc     *proc;       // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int        ncpu;

// PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
	uint edi;
	uint esi;
	uint ebx;
	uint ebp;
	uint eip;
};

enum procstate
{
	UNUSED,
	EMBRYO,
	SLEEPING,
	RUNNABLE,
	RUNNING,
	ZOMBIE
};

#define SHM_MAXNUM 32

//Global kernel structure that tracks the page used as the shared memory. 
struct shm_page{
	char * 				name;		//Name associated with the shared memory page, can be looked up on shm_get 
	int                 cid;        //Container ID that the shared memory page belongs to
	int 				ref_ct;		//How many procs have the shared page mapped into them 
	struct proc *		proc_list[NPROC]; 	//List of the procs that map to this shared memory page 
	int procaddr[NPROC];			//Virtual address that the proc keeps this page 
	uint addr;				 		//Physical address of the shared memory page. If 0, page is freed.		
};

// Per-process state
struct proc {
	uint              sz;            // Size of process memory (bytes)
	pde_t            *pgdir;         // Page table
	char             *kstack;        // Bottom of kernel stack for this process
	enum procstate    state;         // Process state
	int               pid;           // Process ID
	int               cid;           // Container ID
	struct proc *     parent;        // Parent process
	struct trapframe *tf;            // Trap frame for current syscall
	struct context   *context;       // swtch() here to run process
	void             *chan;          // If non-zero, sleeping on chan
	int               killed;        // If non-zero, have been killed
	struct file      *ofile[NOFILE]; // Open files
	struct inode     *cwd;           // Current directory
	char              name[16];      // Process name (debugging)
	int               priority;      // Process priority
	int               maxproc;       // Max number of processes in container
	struct inode *    root;		     // Root directory of container
	struct mutex	*	  Mutexes[16]; //hardcoded value , in lcreate we add the lock to this list
	int 				numMutex; //number of locks availbel to this proc
	int				wait_count;//how many times called cv_wait
	int 			signal_count;//how many times called signal
	int 			sleeping;// if 1 its sleeping, 0 if not
	char *shm_name[SHM_MAXNUM];      // Array of the names of shared memory pages that this process referemces
	void *shm_addr[SHM_MAXNUM]; // Array of virtual addresses that each shared memory page maps to. Indices align
	                            // directly w/shm_name
	struct proc* next;
	int is_fp;
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
