/*
Eugene Sokolov
ECE 357
pset 4 Catgrepmore
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#define BUFFSIZE 4096
#define READ 0
#define WRITE 1

pid_t pidgrep;
pid_t pidmore;
int filesread = 0;
int bytesread = 0;
char *pattern;

void pipe_handler(int signum){
	
	fprintf(stderr, "Interrupt Caught!!\n");
	fprintf(stderr, "Caught: %d\n", signum);

}

void int_handler(int signum){
	
	fprintf(stderr, "Interrupt Caught!!\n");
	fprintf(stderr, "Caught: %d\n", signum);

	fprintf(stderr, "Files read: %d\n", filesread);
	fprintf(stderr, "Bytes processed: %d\n", bytesread);
	fprintf(stderr, "Exit\n");
	
	exit(EXIT_SUCCESS);
	
}

void execfunc(char *inputfile){

	int pipe1[2];
	int pipe2[2];
	int status1;
	int status2;
	
	if(pipe(pipe1) == -1){
		fprintf(stderr, "1. Error pipe1: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if(pipe(pipe2) == -1){
		fprintf(stderr, "2. Error pipe2: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if((pidgrep = fork()) < 0){
		fprintf(stderr, "3. Error forkgrep: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	
	/* Grep child process, put output into read pipe */
	if(pidgrep == 0){
	
		//printf("In grep child\n");	
		
		if((dup2(pipe1[READ], 0)) < 0){
			fprintf(stderr, "5. Error dup2: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if((close(pipe1[READ])) < 0){
			fprintf(stderr, "15. Error closepipe: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if((dup2(pipe2[WRITE], 1)) < 0){
			fprintf(stderr, "6. Error dup2: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if((close(pipe2[WRITE])) < 0){
			fprintf(stderr, "15. Error closepipe: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		if((close(pipe1[WRITE])) < 0){
			fprintf(stderr, "7. Error closepipe: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if((close(pipe2[READ])) < 0){
			fprintf(stderr, "8. Error closepipe: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		execlp("grep", "grep", pattern, NULL);
		fprintf(stderr, "Error exec: %s\n", strerror(errno));

		exit(EXIT_SUCCESS);
	}

	/* More child process, put output into read pipe */
	if((pidmore = fork()) < 0){
		fprintf(stderr, "4. Error forkmore: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if(pidmore == 0){
	
		//printf("In more child\n");	
		
		if((dup2(pipe2[READ], 0)) < 0){
			fprintf(stderr, "9. Error dup2: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if((close(pipe2[READ])) < 0){
			fprintf(stderr, "15. Error closepipe: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if((close(pipe1[READ])) < 0){
			fprintf(stderr, "10. Error closepipe: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if((close(pipe1[WRITE])) < 0){
			fprintf(stderr, "11. Error closepipe: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if((close(pipe2[WRITE])) < 0){
			fprintf(stderr, "12. Error closepipe: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		execlp("more", "more", NULL);
		fprintf(stderr, "Error exec: %s\n", strerror(errno));

		exit(EXIT_SUCCESS);
	}

	/* Parent process */
	if(pidmore > 0 && pidgrep > 0){

		//printf("In parent\n");	
			
		if((close(pipe1[READ])) < 0){
			fprintf(stderr, "13. Error closepipe: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if((close(pipe2[READ])) < 0){
			fprintf(stderr, "14. Error closepipe: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if((close(pipe2[WRITE])) < 0){
			fprintf(stderr, "15. Error closepipe: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		int b, fd;
	
		if((fd = open(inputfile, O_RDONLY)) < 0){
			fprintf(stderr, "16. Error closepipe: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		char *buf = malloc(BUFFSIZE);
		
		while((b = read(fd, buf, BUFFSIZE)) > 0){
			if((write(pipe1[1], buf, b)) < 0){
				fprintf(stderr, "17. Error write: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			bytesread += b;
		}
		
		if((close(fd)) < 0){
			fprintf(stderr, "18. Error closefd: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		waitpid(pidmore, &status1, 0);	
		
		waitpid(pidgrep, &status2, 0);
	}

}

int main(int argc, char *argv[]){

	int i;

	if(argc < 3){
		fprintf(stderr, "Too few arguments");
		exit(EXIT_FAILURE);
	}
		
	signal(SIGINT,int_handler);
	signal(SIGPIPE,pipe_handler);

	pattern = argv[1];

	for(i = 2; i < argc; i++){
		filesread++;		
		execfunc(argv[i]);
	}

	fprintf(stdout, "Reached end of files\n");
	fprintf(stdout, "Files read: %d\n", filesread);
	fprintf(stdout, "Bytes processed: %d\n", bytesread);
	fprintf(stdout, "Exit\n");
		
	return 0;
}
