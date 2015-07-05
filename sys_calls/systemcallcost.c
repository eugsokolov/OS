#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#define iteration 100000000 

void emptyfunction(){
}

void syserrorcheck(char func[], int returnval){

        if(returnval < 0){
                fprintf(stderr, "Error %s\n: %s\n", func, strerror(errno));
                exit(EXIT_FAILURE);
        }

}

int main(){

	int i, j, k;
	struct timespec tps3, tpe3;
	struct timespec tps2, tpe2;
	struct timespec tps1, tpe1;
	long before, after;

/* empty loop */
	printf("empty loop\n");
	syserrorcheck("c1", clock_gettime(CLOCK_REALTIME, &tps1));
	//printf("time before iteration: %ld\n ", before);
	
	for(i = 0; i < iteration; i++){
		;
	}

	syserrorcheck("c1", clock_gettime(CLOCK_REALTIME, &tpe1));
	//printf("time after iteration: %ld\n ", after);

	printf("time: %ld sec\n", tpe1.tv_sec - tps1.tv_sec);
	printf("time: %ld nsec\n", tpe1.tv_nsec - tps1.tv_nsec);
	printf("total %lf\n", (tpe1.tv_sec - tps1.tv_sec)*1000000000+(tpe1.tv_nsec - tps1.tv_nsec)/1000000.0 );
	printf("\n");

/* empty function loop */
	printf("empty function loop\n");
	syserrorcheck("c2", clock_gettime(CLOCK_REALTIME, &tpe2));
	clock_gettime(CLOCK_REALTIME, &tps2);
	//printf("time before iteration: %ld\n ", before);
	
	for(j = 0; j < iteration; j++){
		emptyfunction();
	}

	syserrorcheck("c2", clock_gettime(CLOCK_REALTIME, &tpe2));
	clock_gettime(CLOCK_REALTIME, &tpe2);
	//printf("time after iteration: %ld\n ", after);

	printf("time: %ld sec\n", tpe2.tv_sec - tps2.tv_sec);
	printf("time: %ld nsec\n", tpe2.tv_nsec - tps2.tv_nsec);
	printf("total %lf\n", (tpe2.tv_sec - tps2.tv_sec)*1000000000+(tpe2.tv_nsec - tps2.tv_nsec)/1000000.0 );

	printf("\n");

/* system call loop */
	printf("system call loop\n");
	syserrorcheck("c3", clock_gettime(CLOCK_REALTIME, &tpe3));
	//printf("time before iteration: %ld\n ", before);
	
	for(k = 0; k < iteration; k++){
		getgid();
	}

	syserrorcheck("c3b", clock_gettime(CLOCK_REALTIME, &tpe3));
	//printf("time after iteration: %ld\n ", after);

	printf("time: %ld sec\n", tpe3.tv_sec - tps3.tv_sec);
	printf("time: %ld nsec\n", tpe3.tv_nsec - tps3.tv_nsec);
	printf("total %lf\n", (tpe3.tv_sec - tps3.tv_sec)*1000000000+(tpe3.tv_nsec - tps3.tv_nsec)/1000000.0 );

	printf("\n");
	
	return 0;
}
