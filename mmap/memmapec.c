/* 
Eugene Sokolov
ECE 357, pset 5
Extra Credit, determine page size
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <setjmp.h>
#define BUFFERSIZE 512

jmp_buf jmpbuf;
int fault;
int firstfault;
int secondfault;
int count;
char *addr;
char *addr2;

void sighandler(int signum){

	fprintf(stderr, "Caught signal: %d\n", signum);
	
	count++;		
	if(count == 1)
		firstfault = fault;
	else{
		fprintf(stderr, "pagesize: %d\n", fault-firstfault);
		exit(EXIT_FAILURE);
	}
	
	fprintf(stderr, "fault: %d\n", fault);
	fprintf(stderr, "firstfault: %d\n", firstfault);
	fprintf(stderr, "count: %d\n", count);

	longjmp(jmpbuf, 1);

}

int main(){
	
	/*
	Done: 4096
	printf("size: %d\n", getpagesize());
	*/
	
	sigset_t signal_open;
	signal(SIGSEGV, sighandler);	
	
	setjmp(jmpbuf);
	sigprocmask(SIG_UNBLOCK, &signal_open, NULL);
	sigemptyset(&signal_open);
	sigaddset(&signal_open, SIGSEGV);

	printf("JUMPED 1 \n");
	if(count == 1){
		printf("JUMPED 2 \n");
		addr2 = addr;
		if( (munmap(addr, fault)) < 0){
			fprintf(stderr, "Error munmap: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	if( (addr = mmap(0, fault + 1, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED){
		fprintf(stderr, "Error mmap: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	for(; fault < 100000; fault++){
		addr[fault] = 70;
		if(fault % 1000 == 0)
			printf("fault %d\n", fault);
	}
	
	return 0;
}
