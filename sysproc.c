#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
	return fork();
}

int
sys_exit(void)
{
	exit();
	return 0; // not reached
}

int
sys_wait(void)
{
	return wait();
}

int
sys_kill(void)
{
	int pid;

	if (argint(0, &pid) < 0) return -1;
	return kill(pid);
}

int
sys_getpid(void)
{
	return myproc()->pid;
}

int
sys_sbrk(void)
{
	int addr;
	int n;

	if (argint(0, &n) < 0) return -1;
	addr = myproc()->sz;
	if (growproc(n) < 0) return -1;
	return addr;
}

int
sys_sleep(void)
{
	int  n;
	uint ticks0;

	if (argint(0, &n) < 0) return -1;
	acquire(&tickslock);
	ticks0 = ticks;
	while (ticks - ticks0 < n) {
		if (myproc()->killed) {
			release(&tickslock);
			return -1;
		}
		sleep(&ticks, &tickslock);
	}
	release(&tickslock);
	return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
	uint xticks;

	acquire(&tickslock);
	xticks = ticks;
	release(&tickslock);
	return xticks;
}

int
sys_prio_set(void)
{
	int pid, priority;

	if (argint(0, &pid) < 0) return -1;
	if (argint(1, &priority) < 0) return -1;
	
	return prio_set(pid, priority);
}

int
sys_cm_create_and_enter(void)
{
	return cm_create_and_enter();
}

int
sys_cm_setroot(void)
{
	char *path;
	int   path_len;

	if (argstr(0, &path) < 0 || argint(1, &path_len) < 0) return -1;

	return cm_setroot(path, path_len);
}

int
sys_cm_maxproc(void)
{
	int nproc;

	if (argint(0, &nproc) < 0) return -1;

	return cm_maxproc(nproc);
}
int sys_mut_create(void){
	char *name;
	if (argptr(0, &name, sizeof(char *)) < 0){
		return -1;
	}
	
	return mut_create(name);;
}
int sys_mut_delete(void){
	int muxid;
	if (argint(0, &muxid) < 0 ){
		return -1;
	}
	return mut_delete(muxid);
}

int sys_mut_lock(void){
	int muxid;
	if (argint(0, &muxid) < 0 ){
		return -1;
	}
	return mut_lock(muxid);
}

int sys_mut_unlock(void){
	int muxid;
	if (argint(0, &muxid) < 0 ){
		return -1;
	}
	return mut_unlock(muxid);
}

int sys_c_wait(void){
	int muxid;
	if (argint(0, &muxid) < 0 ){
		return -1;
	}
	return c_wait(muxid);
}
int sys_c_signal(void){
	int muxid;
	if (argint(0, &muxid) < 0 ){
		return -1;
	}
	return c_signal(muxid);
}

char * sys_shm_get(void)
{
	char * name; 
	if(argptr(0, &name, sizeof(name)) < 0)
	{
		return (void *) 0;
	}
	return shm_get(name); 
}

int sys_shm_rem(void)
{
	char * name; 
	if(argptr(0, &name, sizeof(name)) < 0)
	{
		return -1;
	}
	return shm_rem(name);
}

void sys_shm_print(void)
{
	shm_print();
	return;
}

void *
sys_shm_retaddr(void)
{
	char * name; 
	if(argptr(0, &name, sizeof(name)) < 0)
	{
		return (void *) -1;
	}
	return shm_retaddr(name);
}
