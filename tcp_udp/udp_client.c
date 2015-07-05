/*
Eugene Sokolov
ECE 357, problem set 6
UDP client
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
#include <netdb.h>
#define BUF_SIZE 512

void syserrorcheck(char func[], int returnval){

        if(returnval < 0){
                fprintf(stderr, "Error %s\n: %s\n", func, strerror(errno));
                exit(EXIT_FAILURE);
        }

}

int main(int argc, char *argv[]){

	if(argc != 4)
		syserrorcheck("args", -1);

	int s, port;
	char buf[BUF_SIZE] = {0};
	struct sockaddr_in clientside;
	struct hostent *he;
	socklen_t peer_addr_size = sizeof(clientside);	

	port = atoi(argv[2]);
	strcpy(buf, argv[3]);

	syserrorcheck("socket", s = socket(AF_INET, SOCK_DGRAM, 0) );

	clientside.sin_family = AF_INET;
	//serverside.sin_socktype = SOCK_DGRAM;
	clientside.sin_port = htons(port);

	clientside.sin_addr.s_addr = inet_addr(argv[1]);
	if(clientside.sin_addr.s_addr == -1){

		if( (he = gethostbyname(argv[1]) ) < 0 )
			syserrorcheck("gethosebyname", -1);
		else{
			clientside.sin_family = he->h_addrtype;
			memcpy(&clientside.sin_addr.s_addr, he->h_addr, he->h_length);
		}
	}	

	syserrorcheck("sendto", sendto(s, buf, BUF_SIZE, 0 , (struct sockaddr *) &clientside, peer_addr_size) );
		
	syserrorcheck("recvfrom", recvfrom(s, buf, BUF_SIZE, 0 ,(struct sockaddr *) &clientside, &peer_addr_size) );

	fprintf(stderr, "REQUEST: %s\n", buf);
	fprintf(stderr, "IP: %s port: %d\n ", inet_ntoa(clientside.sin_addr), ntohs(clientside.sin_port) );

	syserrorcheck("close", close(s));

	return 0;
}
