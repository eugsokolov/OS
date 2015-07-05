/*
Eugene Sokolov
ECE 357 pset 3, Shell
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

int stream;
int type;
int start;
int script = 0;
char *command[128];
char path[80];
pid_t pid;

int printinfo(){

	struct rusage ru;
	int state;


	printf("Executing command %s\n", command[0]);
	printf("Command returned with return code %d\n", WEXITSTATUS(state));
	printf("consuming %ld real second, %ld user, %ld system\n", ru.ru_utime.tv_sec, ru.ru_utime.tv_usec, (ru.ru_stime.tv_sec/1000000));
	

	if(wait3(&state, 0, &ru) == -1){
		fprintf(stderr, "Error wait3(): %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	return 0;
}

void duper(){

	int fd;

	if(type == 0)
		fd = open(path, O_RDONLY, 0777);
	else if (type == 1)
		fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	else if(type == 2)
		fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0777);

	if(fd < 0){
		fprintf(stderr, "Error opening file: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}		

	if(dup2(fd, stream) < 0){
		fprintf(stderr, "Too Doping: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if(close(fd) < 0){
		fprintf(stderr, "Error closing file: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}		

}

void fileredirect(char token2[]){

	int i;

	/* Open filename, redirect stdin */
	if(token2[0] == '<' ){
		type = 0;
		stream = 0;
		start = 1;
	}

	/* Open/Creat/Trunc and redirect stdout */
	if(token2[0] == '>' && (token2[1] != '>')){
		type = 1;
		stream = 1;
		start = 1;
	}

	/* Open/Creat/Trunc and redirect stderr */
	if(token2[0] == '2' && token2[1] == '>' && token2[2] != '>'){
		type = 1;
		stream = 2;
		start = 2;
	}

	/* Open/Creat/Append and redirect stdout */
	if(token2[0] == '>' && token2[1] == '>'){
		type = 2;
		stream = 1;
		start = 2;
	}

	/* Open/Creat/Append and redirect stderr */
	if(token2[0] == '2' && token2[1] == '>' && token2[2] == '>'){
		type = 2;
		stream = 2;
		start = 3;
	}

	for(i = start; i < (sizeof(path) - start); i++){
		path[i - start] = token2[i]; 
	}

	//printf("path: %s\n", path);

}

int executefunc(){
	
	if( (pid = fork()) < 0){
		fprintf(stderr, "Error forking: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//printf("PID: %d\n", pid);

	/* Child process */
	if(pid == 0){
	
		if(start != 0 )
			duper();

		execvp(command[0], command);

		fprintf(stderr, "Error exec: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Parent process */
	if(pid != 0){
		printinfo();
	}

	return 0;
}


void parse(char *inputline){

	char *token;
	char token2[128];
	int i = 0;
		
	type = 0;
	stream = 0;
	start = 0;

	strtok(inputline, "\n");

	token = strtok(inputline, " \t");

	while(token != NULL){

		sprintf(token2, "%s", token);

		if((token2[0] == '>') || (token2[0] == '<') || (token2[0] == '2' && token2[1] == '>')){

			fileredirect(token2);
		}

		else{
			command[i] = token;
			i++;
		}

		token = strtok(NULL, " \t");
		command[i] = '\0';
	}
	
	executefunc();
	
}

int main(int argc, char *argv[]){

	char inputline[80];
	int fd;

	if(argc > 2){
		fprintf(stderr, "Too many arguments");
		exit(EXIT_FAILURE);
	}
	
	else if(argc == 2){

		script = 1;

		if((fd = open(argv[1], O_RDONLY)) < 0){
			fprintf(stderr, "Error reading: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		if((dup2(fd, 0)) < 0){
			fprintf(stderr, "Error duping: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		if((close(fd)) < 0){
			fprintf(stderr, "Error closing: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		while((fgets(inputline, 128, stdin)) != NULL && errno == 0){	
			if(*inputline == '#' || *inputline == '\n')
				continue;
	
			parse(inputline);
		}
		
		exit(0);

	}
	
	else if(argc == 1){
	
		while(1){
		
		printf("$$");	
		if(fgets(inputline, 128, stdin) == NULL || errno != 0 ){
			fprintf(stderr, "Error reading: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
	
		if(*inputline == '#' || *inputline == '\n')
			continue;
		
		parse(inputline);
		
		}
	}

	return 0;
}

