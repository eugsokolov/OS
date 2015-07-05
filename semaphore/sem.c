/* 
Eugene Sokolov
ECE 357, pset 7
Semaphore.c
*/

#include "sem.h"

void syserrorcheck(char func[], int returnval){

	if(returnval < 0){
		fprintf(stderr, "errno: %d\n", returnval);
		fprintf(stderr, "Error with %s: %s\n",func, strerror(errno) );
		exit(EXIT_FAILURE);
	}

}

void sem_init(struct sem *s, int count){
	
	int i;
	for(i = 0; i < N_PROC - 1; i++)
		s->waiting_procs[i] = 0;
	s->count = count;
	s->proc = 0;
	s->lock = 0;
}

int sem_try(struct sem *s){
	
	if(s->count > 0){

		s->count--;
		s->lock = 0;
		return 1;
	}

	else{
		s->waiting_procs[s->count] = getpid();	
		s->lock = 0;
		return 0;
	}
	
}

void sem_wait(struct sem *s){
	
	sigset_t oldmask, newmask, mask;
	
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);
	sigfillset(&newmask);
	sigdelset(&newmask, SIGUSR1);
	sigdelset(&newmask, SIGINT);
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);
	
	while(tas(&(s->lock)))
		;	
	
	//fprintf(stderr, "count: %d\n", s->count);

	if(s->count > 0){

		s->count--;
		s->lock = 0;
		sigprocmask(SIG_SETMASK, &oldmask, NULL);
		return;
	}

	else{
		s->waiting_procs[s->proc] = getpid();	
		s->proc++;
		s->lock = 0;
		sigfillset(&mask);
		sigdelset(&mask, SIGUSR1);
		sigsuspend(&mask);
		
	}
	
}

void sem_inc(struct sem *s){
		
	sigset_t oldmask, newmask, mask;
	sigfillset(&newmask);
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);
	
	while(tas(&(s->lock)))
		;	
	
	//fprintf(stderr, "count: %d\n", s->count);

	if(s->count > 0){
			
		syserrorcheck("kill sem_inc", kill(s->waiting_procs[s->proc], SIGUSR1) );
		s->proc--;
		//sigprocmask(SIG_SETMASK, &oldmask, NULL);
	}

	else if(s->count == 0){
	
		s->count++;
	//	s->waiting_procs[s->count] = getpid();	
	}
	
	s->lock = 0;

	sigprocmask(SIG_SETMASK, &oldmask, NULL);

}
