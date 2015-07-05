/* Eugene Sokolov
ECE357, pset 6
TCP_RECIEVE
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 512

void syserrorcheck(char func[], int returnval){

	if(returnval < 0){
		fprintf(stderr, "Error %s\n: %s\n", func, strerror(errno));
		exit(EXIT_FAILURE);	
	}

}

int main(int argc, char *argv[]){

	if(argc != 2)
		syserrorcheck("args", -1);
	
	struct sockaddr_in serverside, clientside;
	int s, s2;
	int port;
	int bytesread = 0;
	struct timeval starttime, endtime;
	struct hostent *he;
	socklen_t peer_addr_size = sizeof(struct sockaddr_in);

	port = (atoi(argv[1]));

	syserrorcheck("socket", s = socket(AF_INET, SOCK_STREAM, 0));

	serverside.sin_family = AF_INET;
	serverside.sin_port = htons(port);
	serverside.sin_addr.s_addr = INADDR_ANY;
	
	syserrorcheck("bind", bind(s, (struct sockaddr *)&serverside, peer_addr_size));
	
	syserrorcheck("listen", listen(s, 20));

	syserrorcheck("accept", s2 = accept(s, (struct sockaddr *) &clientside, &peer_addr_size) );

	//take start time
	syserrorcheck("TOD start", gettimeofday(&starttime, NULL));

	int a = 0, b = 0;
	char buf[BUF_SIZE] = {0};
	int wlength = 0;
	while( (b = read(s2, buf, BUF_SIZE)) > 0 ){
 		wlength = 0;
		bytesread = bytesread + b;
		while(wlength < b) {
			a = write(1, buf+wlength, b-wlength);			
			wlength += a;
		}

	} 

	syserrorcheck("shutdown", shutdown(s, 0));
	
	//take end time
	syserrorcheck("TOD end", gettimeofday(&endtime, NULL));
	
	double seconds = endtime.tv_sec - starttime.tv_sec;
	double milliseconds = endtime.tv_usec - starttime.tv_usec;
	double totaltime = seconds + milliseconds/1000000;

	fprintf(stderr, "read: %d bytes in %f seconds\n", bytesread, totaltime);
	fprintf(stderr, "RATE: %f MB/sec\n", (double)bytesread/totaltime);

	fprintf(stderr, "IP address: %s\n", inet_ntoa(clientside.sin_addr));
	fprintf(stderr, "port: %d\n", ntohs(clientside.sin_port));

	if( (he = gethostbyaddr(&clientside.sin_addr.s_addr, sizeof(struct in_addr), AF_INET)) )	
	fprintf(stderr, "reverse DNS lookup successful, host: %s\n", he->h_name);
	else
	fprintf(stderr, "reverse DNS lookup unsuccessful\n");
		
	return 0;
}
