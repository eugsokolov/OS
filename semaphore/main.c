/*
Eugene Sokolov
ECE 357, pset 7
Main.c
*/

#include "sem.h"
#include "fifo.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <setjmp.h>
#include <string.h>

void signalhandler(int sig){
}

int main(){

	struct fifo *fifothedog;
	struct sigaction sigact;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_handler = signalhandler; 
	int i;
	int status;
	
	syserrorcheck("sigaction", sigaction(SIGUSR1, &sigact, NULL));

	fifothedog = mmap(0, sizeof(struct fifo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, 0, 0);
	if(fifothedog == MAP_FAILED)
		syserrorcheck("bad dog", -100);

	fifo_init(fifothedog);

	int fd;
	syserrorcheck("file", fd = open("outfile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666) );
	int fd2;
	syserrorcheck("file", fd2 = open("outfile2.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666) );
	int ppid = getpid();	
	int pids[6] = {0};
	int cpid;
	int myprocess;
	int j, k;
	char buf[32] = {0};
	char buf2[32] = {0};
	int n = 5;
	/* Parent forks, then waits*/
	for(i = 0; i < n; i++){
		if(ppid == getpid()){
			syserrorcheck("fork", cpid = fork());
fprintf(stderr, "forked %d\n", cpid);
			myprocess = i;
		}	
		if(cpid != 0){
			pids[i] = cpid;
		}
		else
			sleep(2);
		
		
	}
	if(ppid == getpid()){
		for(i = 0; i < n; i ++){
			waitpid(pids[i], &status, 0);
fprintf(stderr, "pid[%d] %d\n", i, pids[i]);
		}
	}
	else{
	/* Child process */	
fprintf(stderr, "myprocess : %d\n", myprocess);	
		if(myprocess == 2 ){
			for(j = 0; j < (n-1)*10; j++){
//fprintf(stderr, "read: %lu\n", fifo_rd(fifothedog));
				sprintf(buf,"%lu\n", fifo_rd(fifothedog));
				write(fd, buf, sizeof(buf));
			}
		}
		else
			for(k = 0; k < 10; k++){
				fifo_wr(fifothedog, myprocess * 10 + k);
				sprintf(buf2,"%d\n", myprocess*10+k);
				write(fd2, buf2, sizeof(buf2));
			}		
	}
}
