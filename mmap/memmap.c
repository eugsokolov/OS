/*
Eugene Sokolov
ECE 357, pset 5
Memory-mapped files
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#define BUFFERSIZE 512

void sighandler(int signum){

	fprintf(stderr, "Caught signal: %d\n", signum);
	fprintf(stderr, "signal: %s\n", strsignal(signum));
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){

	signal(SIGSEGV, sighandler);
	signal(SIGBUS, sighandler);

	if(argc != 2){
		fprintf(stderr, "arg problem\n");
		exit(EXIT_FAILURE);
	}

	int fd;
	char *addr = '\0';
	struct stat statbuf;

	// Open input file 
	if( (fd = open(argv[1], O_RDWR, 0666)) < 0){
		fprintf(stderr, "Error opening file: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	// Determine input file size
	if( (fstat(fd, &statbuf)) < 0){
		fprintf(stderr, "Error fstat: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	/* Part A: signal 11 is generated SIGSEGV */
	// MMAP file for read-only access and write to memmap 
	/*
	if( (addr = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED){
		fprintf(stderr, "Error mmap: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	addr[0] = 'x';
	*/	

	/* Part B: Yes the changes are visible immediately */
	// MMAP file for MAP_SHARED and write to memmap, compare before/after	
	/*
	char buf[BUFFERSIZE];
	int i;

	if( (read(fd, buf, statbuf.st_size)) < 0){
		fprintf(stderr, "Error reading: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if( (addr = mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
		fprintf(stderr, "Error mmap: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	// Write to memmap region
	addr[1] = 'a';
	addr[2] = 'b';
	addr[3] = 'c';
	addr[5] = 74;

	// Compare the results, before and after write
	for(i = 0; i < statbuf.st_size - 1; i++){
		if(buf[i] != addr[i])
			printf("%c != %c\n", buf[i], addr[i]);
		else
			printf("%c == %c\n", buf[i], addr[i]);
	
	}
	*/


	/* Part C: No, MAP_PRIVATE creates its own copy, hence fat COW */
	// MMAP file for MAP_PRIVATE and write to memmap, compare before/after	
	char buf[BUFFERSIZE];
	char buf2[BUFFERSIZE];
	int i;

	if( (read(fd, buf, statbuf.st_size)) < 0){
		fprintf(stderr, "Error reading: %s\n", strerror(errno));
		fprintf(stderr, "error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	
	if( (addr = mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED){
		fprintf(stderr, "Error mmap: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	// Write to memmap region
	addr[1] = 'a';
	addr[2] = 'b';
	addr[3] = 'c';
	addr[5] = 74;

	lseek(fd, 0, SEEK_SET);

	if( (read(fd, buf2, statbuf.st_size)) < 0){
		fprintf(stderr, "Error reading: %s\n", strerror(errno));
		fprintf(stderr, "error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	// Compare the results, before and after write
	for(i = 0; i < statbuf.st_size - 1; i++){
		if(buf[i] != buf2[i])
			printf("%c != %c\n", buf[i], buf2[i]);
		else
			printf("%c == %c\n", buf[i], buf2[i]);
	
	}
	/*
	*/


	/* Part D: No, there are no changes as the kernel has no way of knowing you wrote past the specified limit */
	// MMAP file for MAP_SHARED and write one byte beyond, does the size change?
	/*
	if( (addr = mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
		fprintf(stderr, "Error mmap: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	int before, after;

	if( (fstat(fd, &statbuf)) < 0){
		fprintf(stderr, "Error fstat: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	before = statbuf.st_size - 1;

	// Write dummy byte at last location 
	addr[statbuf.st_size - 1] = 'x';	
	
	if( (fstat(fd, &statbuf)) < 0){
		fprintf(stderr, "Error fstat: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	after = statbuf.st_size - 1;
	printf("before: %d; after %d\n", before, after);
	*/

	/* Part E: Sparse Files, creates a 'hole' in the file represented as '^@' */
	/*
	if( (addr = mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
		fprintf(stderr, "Error mmap: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	int before, after;

	if( (fstat(fd, &statbuf)) < 0){
		fprintf(stderr, "Error fstat: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	before = statbuf.st_size - 1;

	// Write dummy byte at last location 
	addr[before] = 'x';	
	if( (lseek(fd, statbuf.st_size-1+5, SEEK_SET)) < 0){
		fprintf(stderr, "Error lseek: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if( (write(fd, "x", 1)) < 0){
		fprintf(stderr, "Error write: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if( (fstat(fd, &statbuf)) < 0){
		fprintf(stderr, "Error fstat: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	after = statbuf.st_size - 1;
	printf("before: %d; after %d\n", before, after);
	*/

	/* Part F: Small file that is memmapped two pages long */
	/*
	int twopagemap = 8192;
	
	if( (addr = mmap(0, twopagemap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
		fprintf(stderr, "Error mmap: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// Case 1: Access 1st page, writes 'x' with no errors
	printf("Writing to addr[2]\n");
	addr[2] = 'x';
	printf("Wrote to addr[2]\n");
	// Case 1: Access 2nd page, SIGBUS
	printf("Writing to addr[8000]\n");
	addr[8000] = 'a';
	*/

	// Unmap and close 
	if( (munmap(addr, statbuf.st_size)) < 0){
		fprintf(stderr, "Error munmap: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if( (close(fd)) < 0){
		fprintf(stderr, "Error write: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	return 0;
}
