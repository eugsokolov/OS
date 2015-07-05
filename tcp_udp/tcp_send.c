/* Eugene Sokolov
ECE357, pset 6
TCP_SEND
*/

//My system is little endian
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
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

	if(argc != 3)
		syserrorcheck("args", -1);

	int s;
	struct sockaddr_in serverside;
	struct timeval starttime, endtime;
	struct hostent *he;
	int port;
	int bytesread = 0;
	struct linger lingerer;
	
	port = atoi(argv[2]);
	
	syserrorcheck("socket", s = socket(AF_INET, SOCK_STREAM , 0) );

	serverside.sin_family = AF_INET;
	serverside.sin_port = htons(port);
	serverside.sin_addr.s_addr = inet_addr(argv[1]);

	if(serverside.sin_addr.s_addr == -1){

		if( (he = gethostbyname(argv[1])) < 0 )
			syserrorcheck("host", -1);
		else{
			serverside.sin_family = he->h_addrtype;
			memcpy(&serverside.sin_addr.s_addr, he->h_addr, he->h_length);
		}
	}
	
	lingerer.l_onoff = 1;
	lingerer.l_linger = 10;

	syserrorcheck("setsockopt", setsockopt(s, SOL_SOCKET, SO_LINGER, &lingerer, sizeof(lingerer)) );

	syserrorcheck("connect", connect(s, (struct sockaddr *)&serverside, sizeof(serverside)) );

	//take start time
	syserrorcheck("TOD start", gettimeofday(&starttime, NULL) );

	int a = 0, b = 0;
	char buf[BUF_SIZE] = {0};
	int wlength;
	while( (b = read(0, buf, BUF_SIZE)) > 0 ){
 		wlength = 0;
		bytesread = bytesread + b;
		while(wlength < b) {
			a = write(s, buf+wlength, b-wlength);			
			wlength += a;
		}

	} 

	syserrorcheck("shutdown", shutdown(s, 1));

	//take end time	
 	syserrorcheck("TOD end", gettimeofday(&endtime, NULL));
	
	double seconds = endtime.tv_sec - starttime.tv_sec;
        double milliseconds = endtime.tv_usec - starttime.tv_usec;
        double totaltime = seconds + milliseconds/1000000;

        fprintf(stderr, "bytes read: %d\n", bytesread);
        fprintf(stderr, "in %f MB/sec\n", bytesread/(1000000 * totaltime));
	
	return 0;
}
