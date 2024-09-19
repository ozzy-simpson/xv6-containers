struct stat;
struct rtcdate;

// system calls
int   fork(void);
int   exit(void) __attribute__((noreturn));
int   wait(void);
int   pipe(int *);
int   write(int, void *, int);
int   read(int, void *, int);
int   close(int);
int   kill(int);
int   exec(char *, char **);
int   open(char *, int);
int   mknod(char *, short, short);
int   unlink(char *);
int   fstat(int fd, struct stat *);
int   link(char *, char *);
int   mkdir(char *);
int   chdir(char *);
int   dup(int);
int   getpid(void);
char *sbrk(int);
int   sleep(int);
int   uptime(void);
int   prio_set(int, int);
int   cm_create_and_enter(void);
int   cm_setroot(char *, int);
int   cm_maxproc(int);


//module 3
int mut_create(char * );
int mut_delete(int);
int mut_lock(int );
int mut_unlock(int );
int c_wait(int);
int c_signal(int);

// ulib.c
int   stat(char *, struct stat *);
char *strcpy(char *, char *);
char *strncpy(char *, char *, int);
void *memmove(void *, void *, int);
char *strchr(const char *, char c);
int   strcmp(const char *, const char *);
void  printf(int, char *, ...);
char *gets(char *, int max);
uint  strlen(char *);
void *memset(void *, int, uint);
void *malloc(uint);
void  free(void *);
int   atoi(const char *);

//vm.c 
char *shm_get(char * name); 
int shm_rem(char * name); 
void shm_print(void);
void*  shm_retaddr(char * name);
int mutex_create(char *);
int mutex_delete(int);
int mutex_lock(int );
int mutex_unlock(int );
void cv_wait(int );
void cv_signal(int);