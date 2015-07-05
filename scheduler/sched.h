/*
Eugene Sokolov
ECE 357 pset 7
sched.h
*/

#ifndef SCHED_H
#define SCHED_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "savectx.h"

/* Types of defined processes */
#define SCHED_NPROC 3
#define SCHED_READY 10
#define SCHED_RUNNING 5
#define SCHED_SLEEPING 4
#define SCHED_ZOMBIE 3

/* Use priority scale similar to CFS */
#define LOW_PRIORITY 0
#define DEFAULT_PRIORITY 20
#define HIGH_PRIORITY 39 

#define STACK_SIZE 65536 /* Use stack size of 64K */
#define TICK_RATE 10 /* Slow tick rate to 100msec */

struct sched_proc{
	pid_t pid; /* pid of process */
	int state; /* state of process as define above(ZOMBIE, SLEEPING, etc) */
	int active; /* 1 active; 0 inactive */
	int exitcode; /* exit code of process */
	int cputime; /* time taken by cpu on this process */
	char *stackaddr; /* address of stack */
	int priority; /* Value from 0-40 as defined above */
	int vruntime; /* Value of runtime neediness */
	int sleeping; /* 1 asleep; 0 awake */
	int waiting; /* 1 waiting, 0 non */	
	struct savectx ctx; /* used to move the stack registers correctly */
	struct sched_proc *parent; /* parent process */
	struct sched_proc *children[SCHED_NPROC]; /* children process */
};

struct sched_proc *INIT; /* the INIT process */
struct sched_proc *currentproc, *proc_list[SCHED_NPROC];
int lastusedpid; /* used to keep track of pids used */
int clockticks; /* used to count total tick of cpu */
int NEED_RESCHED; /* used to resched processess */
 
void sched_init(void (*init_fn)());
pid_t assignnewpid();
void inserttoqueue(struct sched_proc *child);
char* getnewstackaddr();
int sched_wait();
void addtorunqueue(pid_t pid);
void removetorunqueue(pid_t pid);
void picknexttask();
int sched_fork();
int sched_exit(int code);
//int sched_wait(int *exit_code);
int sched_nice(int niceval);
pid_t sched_getpid();
pid_t sched_getppid();
int sched_gettick();
void sched_ps();
int sched_switch();
int sched_tick();
void BUG();

#endif
