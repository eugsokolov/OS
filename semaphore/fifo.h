#ifndef FIFO_H
#define FIFO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/mman.h>

#define BUFSIZE 64
#define N_PROC 64

volatile struct fifo{
	char buf[BUFSIZE];
	int next_write, next_read;
	int item_count;
	pid_t waiting_procs[N_PROC];
	//struct sem *sem;
	struct sem *semwrite;
	struct sem *semread;
	struct sem *semmutex;
}fifo;

void fifo_init(struct fifo *f);
void fifo_wr(struct fifo *f, unsigned long d);
unsigned long fifo_rd(struct fifo *f);

#endif
