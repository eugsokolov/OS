/*
Eugene Sokolov
pset 6 ec
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#define BUF_SIZE 512

void errcheck(char func[], int retval){
	
/*	if(retval == 0){
		fprintf(stderr, "Error %s (returned 0) %s\n", func, strerror(errno));
		exit(EXIT_FAILURE);
	}
*/
	if(retval < 0){
		fprintf(stderr,"Error %s: %s\n", func, strerror(errno));
		fprintf(stderr, "errno %d\n", errno);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[]){

	if(argc != 2)
		errcheck("args", -1);

	struct sockaddr_in server, server2;
	socklen_t len = sizeof(struct sockaddr_in);
	int s, s2, s3, port;
	char input[BUF_SIZE] = {0};
	char buf[BUF_SIZE] = {0};
	char buf2[BUF_SIZE] = {0};
	struct hostent *he;
	char *hostname = NULL;

	port = atoi(argv[1]);

	errcheck("socket", s = socket(AF_INET, SOCK_STREAM, 0) );

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;

	errcheck("bind", bind(s, (struct sockaddr *) &server, len) );

	errcheck("listen", listen(s, 20) );

	errcheck("accept", s2 = accept(s, (struct sockaddr *) &server2, &len) );

	errcheck("read input", read(s2, input, BUF_SIZE));

	strtok(input, "\n");
	hostname = strtok(input, " ");
	port = atoi(strtok(NULL, " "));
	
	errcheck("socket", s3 = socket(AF_INET, SOCK_STREAM, 0) );

	server2.sin_family = AF_INET;
	server2.sin_port = htons(port);
	server2.sin_addr.s_addr = inet_addr(hostname);
	
	if( (he = gethostbyname(hostname)) < 0){
		errcheck("gethostbyname", -1);
	}
	else{
		server2.sin_family = he->h_addrtype;
		memcpy(&server2.sin_addr.s_addr, he->h_addr, he->h_length);
	}

	printf("IP: %s\n", inet_ntoa(server.sin_addr));
	printf("IP2: %s\n", inet_ntoa(server2.sin_addr));
	printf("port: %d\n", ntohs(server2.sin_port));

	errcheck("connect", connect(s3, (struct sockaddr *) &server2, len) );

	int b, a;
	int ready;
	struct pollfd fds[BUF_SIZE];
	int fdout;

	errcheck("open", fdout = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777));
	
	fds[0].fd = s2;
	fds[0].events = POLLIN | POLLPRI;
	
	fds[1].fd = s3;
	fds[1].events = POLLIN | POLLPRI;
	
	while(1){	
	errcheck("poll", ready = poll(fds, 2, 10000 ));

	if(fds[0].revents & POLLIN){

		errcheck("read s2->s3" , b = read(s2, buf2, BUF_SIZE ) );
		errcheck("write s2->s3" , write(s3, buf2, b ) );
		
		/* log file */
		errcheck("write out" , write(fdout,"USER2: \n" , 8 ) );
		errcheck("write out" , write(fdout, buf, a ) );
		errcheck("write out" , write(fdout,"\n\n" , 2 ) );

	}
	if(fds[0].revents & POLLPRI){
		printf("OOB\n");

	}
	if(fds[1].revents & POLLIN){

		errcheck("read s3->s2" , a = read(s3, buf, BUF_SIZE ) );
		errcheck("write s3->s2" , write(s2, buf, a ) );
		
		/* log file */
		errcheck("write out" , write(fdout,"USER2: \n" , 8 ) );
		errcheck("write out" , write(fdout, buf, a ) );
		errcheck("write out" , write(fdout,"\n\n" , 2 ) );
		
	}
	}

	errcheck("close", close(fdout));

	return 0;
}

