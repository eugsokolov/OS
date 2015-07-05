/*
Eugene Sokolov
ECE 357, pset 9
sched.c
*/

#include "sched.h"

void sched_init(void (*init_fn)()){

	/* Set up INIT */
	NEED_RESCHED = 0;
	clockticks = 0;	
	lastusedpid = 0;

	if(!(INIT = malloc(sizeof(struct sched_proc))) ){
		fprintf(stderr, "sched_init: error malloc \n");
		exit(EXIT_FAILURE);
	}

	INIT->pid = 1;
	INIT->state = SCHED_RUNNING;
	INIT->active = 1;
	INIT->exitcode = 0;
	INIT->cputime = 0;
	INIT->stackaddr = getnewstackaddr();
	INIT->priority = DEFAULT_PRIORITY;
	INIT->vruntime = 10;
	INIT->sleeping = 0;
	INIT->parent = INIT;
	INIT->waiting = 0;

	int k;
	for(k = 0; k<4; k++){
	if(!(INIT->children[k] = malloc(sizeof(struct sched_proc))) ){
		fprintf(stderr, "sched_init: error malloc \n");
		exit(EXIT_FAILURE);
	}
	INIT->children[1] = NULL;
	}

	int i;
	for(i = 0; i < SCHED_NPROC; i++)
		proc_list[i] = NULL;
	currentproc = INIT;
	/* set up savectx stuff */
	savectx(&(INIT->ctx));
	INIT->ctx.regs[JB_SP] = INIT->stackaddr + STACK_SIZE;
	INIT->ctx.regs[JB_BP] = INIT->stackaddr + STACK_SIZE;
	INIT->ctx.regs[JB_PC] = init_fn;
	
	/* Set up alarm stuff */
	struct sigaction sa;
	sa.sa_flags = 0;
	//sa.sa_handler = (void(*) (int))sched_tick;
	sa.sa_handler = (void (*) )sched_tick;
	sigemptyset(&sa.sa_mask);
	if(sigaction(SIGVTALRM, &sa, NULL)){
		fprintf(stderr, "sched_init: error sigaction %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sigaction(SIGABRT, &sa, NULL)){
		fprintf(stderr, "sched_init: error sigaction %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* set up timer stuff */
	struct itimerval itv;
	itv.it_value.tv_sec = 0;
	itv.it_value.tv_usec = TICK_RATE;
	itv.it_interval.tv_sec = 0;
	itv.it_interval.tv_usec = TICK_RATE;
	setitimer(ITIMER_VIRTUAL, &itv, NULL);
	
	restorectx(&(INIT->ctx), 0);

	fprintf(stderr, "INIT DONE\n");
}

char* getnewstackaddr(){

	char *n;
	if((n = mmap(0, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0)) == MAP_FAILED){
		fprintf(stderr, "error newstackaddr %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return n;
}

void picknexttask(){

	int i, j;
	int cp = currentproc->vruntime;
	struct sched_proc *nexttask;
	int temp;
fprintf(stderr, "picking task\n\n");
	for(i = 0; i < SCHED_NPROC; i++){
			
		proc_list[i]->vruntime = temp;
		if(temp > cp ){
			j = i;
		}

	}
	
	currentproc->state = SCHED_SLEEPING;
	currentproc->sleeping = 1;
		
	currentproc = proc_list[j];
}

pid_t assignnewpid(){

	int i;
	
	for(i = lastusedpid; i < SCHED_NPROC; i++){
		if(!proc_list[i]){
			lastusedpid = i+1;
			return i+1;
		}

	}

}

void inserttoqueue(struct sched_proc *child){

	int i = 0;
	for(i = 0; i < SCHED_NPROC; i++){
		if(proc_list[i] == NULL){
			proc_list[i] = child;
			break;	
		}
	}

}

int sched_fork(){

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &mask, NULL);
	
	int cpid;
	struct sched_proc *child;
	
	if(!(child = malloc(sizeof(struct sched_proc))) ){
		fprintf(stderr, "sched_fork: error malloc chidl\n ");
		return -1;
	}

	if( (cpid = assignnewpid()) < 0){
		fprintf(stderr, "sched_fork: error new pid \n ");
		return -1;
	}
	
	child->pid = cpid;
	child->state = SCHED_READY;
	child->active = 1;
	child->exitcode = 0;
	child->cputime = 0;
	child->stackaddr = getnewstackaddr();
	child->priority = DEFAULT_PRIORITY;
	child->vruntime = currentproc->vruntime;
	child->sleeping = 0;
	child->parent = currentproc;
	currentproc->children[cpid] = child;

	currentproc->waiting = 1;

	/* copy register values */
	if(!savectx(&child->ctx)){
		unsigned long adj = child->stackaddr - currentproc->stackaddr;
		memcpy(child->stackaddr, currentproc->stackaddr, STACK_SIZE);
		adjstack(child->stackaddr, child->stackaddr + STACK_SIZE, adj);
		child->ctx.regs[JB_SP] += adj;
		child->ctx.regs[JB_BP] += adj;
	}
	else{
		sigprocmask(SIG_UNBLOCK, &mask, NULL);	
		return -1;
	}
	
	inserttoqueue(child);	
	sched_ps();
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	return cpid;
}

int sched_exit(int code){

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	INIT->waiting=0;
	currentproc->parent->waiting = 0;
	currentproc->state = SCHED_ZOMBIE;
	currentproc->exitcode = code;
	currentproc->sleeping = 0;
fprintf(stderr, "exited with code: %d\n", code);
	
	//wake up process in sched_wait()
	if(currentproc->parent->state == SCHED_SLEEPING){
		
		currentproc->parent->state = SCHED_READY;
		currentproc->parent->active = 1;
		currentproc->parent->sleeping = 0;
	}

	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	sched_switch();
	BUG();
}

int sched_wait(){

	if(INIT->waiting == 0)
		return -1;
	
	else{ 
		currentproc->state = SCHED_SLEEPING;
		if(clockticks % 20 == 3)
			INIT->waiting = 0;
		return currentproc->pid;
	}
}

int sched_nice(int niceval){

	if(niceval > HIGH_PRIORITY || niceval < LOW_PRIORITY){
		fprintf(stderr, "wrong niceval, not in range \n");
		return -1;
	}

	//only root process can increase priority
	if(sched_getpid() != 0){ 
	
		if(currentproc->priority > niceval){
			fprintf(stderr, "wrong niceval, not root \n");
			return -1;
		}
		else{
			currentproc->priority = niceval;
			return 0;
		}//angelina jolie yum
	}
	else{
		currentproc->priority = niceval;
		return 0; 
	}
}


pid_t sched_getpid(){
	return currentproc->pid;
}

pid_t sched_getppid(){
	return currentproc->parent->pid;
}

int sched_gettick(){
	return clockticks;
}

void sched_ps(){

	fprintf(stdout, "Information:\n");
	
	int i;
	for(i = 0; i< SCHED_NPROC; i++){
		if(proc_list[i] != NULL){
		
			fprintf(stdout, "pid:%d\tparent:%d\tstate:%d\taddr:%p\tvruntime:%d \n",
				proc_list[i]->pid,
				proc_list[i]->parent->pid,
				proc_list[i]->state,
				proc_list[i]->stackaddr,
				proc_list[i]->vruntime);
			fprintf(stdout, "\t niceval:%d\t   \n",
				proc_list[i]->priority);
				fprintf(stdout, "\tniceval:%d\tcpu:%d\tclocktick:%d\t  \n",
				proc_list[i]->priority,
				proc_list[i]->cputime,
				clockticks );

			if(proc_list[i]->state == SCHED_SLEEPING){
				fprintf(stdout, "sleeping\n");
				fprintf(stdout, "%d\t %d\t %d\t  \n",
				proc_list[i]->priority,
				proc_list[i]->cputime,
				clockticks );
			}

		fprintf(stderr, "waiting %d\n", INIT->waiting);

			if(proc_list[i]->state == SCHED_ZOMBIE){
				fprintf(stdout, "zombie \n");
				fprintf(stdout, "exit code %d\n", proc_list[i]->exitcode);


			}
			
		}
	}

}



int sched_switch(){

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	struct sched_proc *next, *prev;
fprintf(stderr, "Switching\n\n\n");	
	if(NEED_RESCHED){

		NEED_RESCHED = 0;
		currentproc->state = SCHED_READY;
		currentproc->priority = DEFAULT_PRIORITY;
		currentproc->active = 1;
	
		picknexttask();
		next->state = SCHED_RUNNING;
		prev = currentproc;
		currentproc = next;
	}
	
	sched_ps();
	INIT->waiting=0;	
	if(!savectx(&prev->ctx))
		restorectx(&currentproc->ctx, 1);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

}


int sched_tick(){

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &mask, NULL);
	fprintf(stdout, "tick\n\n\n\n");

	currentproc->cputime++;
	clockticks++;

	//int temp = currentproc->priority * .85;
	//temp++;
	//currentproc->vruntime = temp;
	currentproc->vruntime++;	

	NEED_RESCHED = 0;

	
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	sched_switch();
}

void BUG(){
	fprintf(stderr, "COCKAROACH\n\n");
	exit(EXIT_FAILURE);
}
