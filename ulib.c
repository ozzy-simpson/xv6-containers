#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

char *
strcpy(char *s, char *t)
{
	char *os;

	os = s;
	while ((*s++ = *t++) != 0)
		;
	return os;
}

char *
strncpy(char *s, char *t, int n)
{
	char *os;

	os = s;
	while ((*s++ = *t++) != 0 && --n > 0)
		;
	return os;
}

int
strcmp(const char *p, const char *q)
{
	while (*p && *p == *q) p++, q++;
	return (uchar)*p - (uchar)*q;
}

uint
strlen(char *s)
{
	int n;

	for (n = 0; s[n]; n++)
		;
	return n;
}

void *
memset(void *dst, int c, uint n)
{
	stosb(dst, c, n);
	return dst;
}

char *
strchr(const char *s, char c)
{
	for (; *s; s++)
		if (*s == c) return (char *)s;
	return 0;
}

char *
gets(char *buf, int max)
{
	int  i, cc;
	char c;

	for (i = 0; i + 1 < max;) {
		cc = read(0, &c, 1);
		if (cc < 1) break;
		buf[i++] = c;
		if (c == '\n' || c == '\r') break;
	}
	buf[i] = '\0';
	return buf;
}

int
stat(char *n, struct stat *st)
{
	int fd;
	int r;

	fd = open(n, O_RDONLY);
	if (fd < 0) return -1;
	r = fstat(fd, st);
	close(fd);
	return r;
}

int
atoi(const char *s)
{
	int n;

	n = 0;
	while ('0' <= *s && *s <= '9') n= n * 10 + *s++ - '0';
	return n;
}

void *
memmove(void *vdst, void *vsrc, int n)
{
	char *dst, *src;

	dst = vdst;
	src = vsrc;
	while (n-- > 0) *dst++= *src++;
	return vdst;
}

int mutex_create(char *name){
	//mut_create
	return mut_create(name);
}
int mutex_delete(int muxid){
	return mut_delete(muxid);
}
int mutex_lock(int muxid){
	//mut_lock
	return mut_lock(muxid);
}
int mutex_unlock(int muxid){
	//mut_unlock
	return mut_unlock(muxid);
}
void cv_wait(int muxid){
	//cv_wait
	c_wait(muxid);
}
void cv_signal(int muxid){
	//cv_signal
	c_signal(muxid);
}