/*
Eugene Sokolov
ECE 357 pset 7
FIFO
*/

#include "fifo.h"
#include "sem.h"

void fifo_init(struct fifo *f){

	f->next_write = 0;
	f->next_read = 0;
	f->item_count = 0;
	int i;
	for(i = 0; i < N_PROC; i++)
		f->waiting_procs[i] = 0;

	f->semwrite = mmap(0, sizeof(struct sem), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
	if(f->semwrite == MAP_FAILED)
		syserrorcheck("mmap semwrite", -100);
	f->semread = mmap(0, sizeof(struct sem), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
	if(f->semread == MAP_FAILED)
		syserrorcheck("mmap semread", -100);
	f->semmutex = mmap(0, sizeof(struct sem), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
	if(f->semmutex == MAP_FAILED)
		syserrorcheck("mmap semmutex", -100);

	sem_init(f->semread, 0);
	sem_init(f->semwrite, BUFSIZE);
	sem_init(f->semmutex, 1);
}

void fifo_wr(struct fifo *f, unsigned long d){

	sem_wait(f->semwrite);		
	sem_wait(f->semmutex);
	
	//fprintf(stderr, "countwrite: %d\n", f->semwrite->count);
	//fprintf(stderr, "countmut: %d\n", f->semmutex->count);
	
	fprintf(stderr, "writing D: %lu\n", d);
	f->buf[f->next_write++] = d;
	f->next_write %= BUFSIZE; //initialize
	f->item_count++;
	
	sem_inc(f->semmutex);
	sem_inc(f->semread);

}

unsigned long fifo_rd(struct fifo *f){

	unsigned long d;

	sem_wait(f->semread);		
	sem_wait(f->semmutex);

	//fprintf(stderr, "countread: %d\n", f->semread->count);
	//fprintf(stderr, "countmut: %d\n", f->semmutex->count);

	//read while item_count < size of write
	d = f->buf[f->next_read++];
	f->next_read %= BUFSIZE; //initialize
	f->item_count--;

	
	sem_inc(f->semmutex);
	sem_inc(f->semwrite);

	return d;
}
