/*
Eugene Sokolov, problem set 1
ECE 357 Fall 2013
*/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

extern char *optarg;
extern int opting;

int main(int argc, char *argv[]){

	int opt, fdin, fdout, c, d;
	int buffsize = 1024;
	char * outfile;
	char buf[buffsize];
	outfile = "1";

// Handle the input flags
	while((opt = getopt(argc,argv,"b:o:")) != -1){
	
	//printf("optarg:%s\n", optarg);

		switch(opt){
	
		case 'b':
			buffsize = atoi(optarg);
			break;

		case 'o':
			outfile = optarg;
			break;	
		
		default:
			fprintf(stderr, "Usage: %s [-b bytes] [-o outfile]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	fdout = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);

//Handle buffer error
	if(buffsize < 1 || buffsize > 256001){
		fprintf(stderr, "Buffer input not appropriate\n");
		exit(EXIT_FAILURE);
	}
	
	printf("buffersize=%i, outfile=%s\n", buffsize, outfile);

/*	if( !(buf=malloc(buffsize))){
		fprintf(stderr, "Can't allocate byte buffer");
	}
*/
// Read input and write to output
	while(optind < argc ){
	
		printf("optind:%d, argc:%d\n", optind, argc);
 
		if(optind >= argc) //no input files
			fdin = 1;
		else
			fdin = open(argv[optind], O_RDONLY);
		

		if(fdin < 0 || fdout < 0 ){
			fprintf(stderr, "File error\n");
			exit(EXIT_FAILURE);
		}
		
		printf("fdin= %d, fdout= %d\n", fdin, fdout);
		

		while(( c = read(fdin, buf, buffsize))){
			
			if(strcmp(outfile, "1"))
				d = write(fdout, buf, c);
			else
				d = write(1, buf, c);

		//read error check
			if(c < 0){
				fprintf(stderr, "Can't open file for reading");
			}

		//Partial write check
			if( c != d){
			fprintf(stderr, "Partial write error\n");
			exit(EXIT_FAILURE);
			}	
		}
	
		optind++;
		if(optind == argc)
			break;
	
		if(close(fdout) < 0){
			fprintf(stderr, "Failed to close output");
		}
		
	}

	close(fdout);
	return 0;	
}
