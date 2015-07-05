/*
Eugene Sokolov
ECE 357, problem set 6
UDP server
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 512
#define READ 0
#define WRITE 1

void syserrorcheck(char func[], int returnval){

        if(returnval < 0){
                fprintf(stderr, "Error %s: %s\n", func, strerror(errno));
                exit(EXIT_FAILURE);
        }

}

char* message(int type){

	char *msg = malloc(BUF_SIZE * sizeof(char));
	pid_t pid;
	int status;
	int fd[2];
	
	syserrorcheck("pipe", pipe(fd));
	syserrorcheck("fork", pid = fork());

	if(pid == 0){
	
		syserrorcheck("close", close(fd[READ]));
		syserrorcheck("dup2", dup2(fd[WRITE], 1));

		if (type == 1){
			execlp("date", "date", NULL);
			fprintf(stderr, "exec error %s\n", strerror(errno));
		}
		else if(type == 2){
			execlp("uptime", "uptime", NULL);
			fprintf(stderr, "exec error %s\n", strerror(errno));	
		}
		
	}
	else if(pid > 0){
		
		syserrorcheck("close", close(fd[WRITE]));
	
		syserrorcheck("read", read(fd[READ], msg, BUF_SIZE));
		
		syserrorcheck("close", close(fd[READ]));
		
		waitpid(pid, &status, 0);
	
	}

	return msg;
}

int main(int argc, char *argv[]){

	if(argc != 2)
		syserrorcheck("args", -1);

	int s, port;
	int type = 0;
	char buf[BUF_SIZE] = {0};
	char *msg = NULL;
	struct sockaddr_in serverside, clientside;
	socklen_t peer_addr_size = sizeof(clientside);	

	port = atoi(argv[1]);
	
	syserrorcheck("socket", s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) );

	serverside.sin_family = AF_INET;
	//serverside.sin_socktype = SOCK_DGRAM;
	serverside.sin_port = htons((unsigned short)port);
	serverside.sin_addr.s_addr = INADDR_ANY;

	syserrorcheck("bind", bind(s, (struct sockaddr *) &serverside, sizeof(serverside)) );

	while(1){

	syserrorcheck("recvfrom", recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *) &clientside, &peer_addr_size) );
	
	if(strcmp(buf, "uptime") == 0)
		type = 2; 
	else if(strcmp(buf, "date") == 0)
		type = 1;

	msg = message(type);
	
	syserrorcheck("sendto", sendto(s, msg, BUF_SIZE, 0, (struct sockaddr *) &clientside, peer_addr_size) );
	
	fprintf(stderr, "IP: %s port: %d\n ", inet_ntoa(clientside.sin_addr), ntohs(clientside.sin_port) );
	
	}

	syserrorcheck("close", close(s));

	return 0;
}
