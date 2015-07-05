/* 
Eugene Sokolov
simplified find command,  problem set 2
ECE 357 Fall 2013
*/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>

extern char *optarg;
extern int opting;

char* fileperm(mode_t mode){

    int i;
    char *p;
    static char pm[10];
    p = pm;
    strcpy(pm, "---------");

	switch (mode & S_IFMT) {
    		case S_IFREG:
        		break;
    	 case S_IFDIR:
        	*p = 'd';
   	 case S_IFCHR:
 	        *p = 'c';
	 case S_IFBLK:
	        *p = 'b';
   	 case S_IFLNK:
   		*p = 'l';
   	 case S_IFIFO:
   	     	*p = 'p';
   	 case S_IFSOCK:
   	     	*p = 's';
   	 }

	for (i = 0; i < 3; i++) {
       		if (mode & (S_IREAD >> i*3))
            		*p = 'r';
       		p++;

       		if (mode & (S_IWRITE >> i*3))
            		*p = 'w';
        	p++;

        	if (mode & (S_IEXEC >> i*3))
            		*p = 'x';
        	p++;
    	}

	if ((mode & S_ISUID) != 0)
        	pm[2] = 's';

    	if ((mode & S_ISGID) != 0)
        	pm[5] = 's';

    	if ((mode & S_ISVTX) != 0)
        	pm[8] = 't';

    return(pm);


}

int info(char *filename, char *pathname, struct stat filestat, int mtime, char* user){

	struct passwd *pwd;
	struct group *grp;

	stat(pathname, &filestat);

	printf("%04x/", (unsigned int)filestat.st_dev);
	printf("%-7i ", (int)filestat.st_ino);

	//printf("%lo ", (unsigned long)filestat.st_mode );
	printf("%s ", fileperm(filestat.st_mode));
	
	printf("%2i ", (int)filestat.st_nlink);
	
	if( (pwd = getpwuid(filestat.st_uid)) != NULL)
		printf("%-8.8s ", pwd->pw_name);
	else
		printf("%-8d ", filestat.st_uid);
	
	if( (grp = getgrgid(filestat.st_gid)) != NULL)
		printf("%-8.8s ", grp->gr_name);
	else
		printf("%-8d ", filestat.st_gid);
	 
	//print out block size for block special and char special node
	if(((filestat.st_mode && S_IFMT) == S_IFBLK ) || ((filestat.st_mode && S_IFMT) == S_IFCHR))
		printf("%10x ", (unsigned int)filestat.st_rdev);
	else
		printf("%10i ", (unsigned int)filestat.st_size);

	printf("%s ", ctime(&(filestat.st_mtime)));
	printf("%s\n", filename);

	return 0;

}

int finder(char *searchinput, int mtime, char* user, char* target, int traverse){

	DIR *dirp;
	struct dirent *dir;
	struct stat filestat;
	struct stat tr;
	char buf[128];
	
	if ((dirp = opendir(searchinput)) == NULL){
		fprintf(stderr, "Error opening dir: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	while( (dir = readdir(dirp)) ){
		if(strcmp(dir->d_name, ".") == 0 ||
		   strcmp(dir->d_name, "..") == 0)
		continue;

		char *fullpath = malloc(strlen(dir->d_name) + strlen(searchinput) + 2);
		sprintf(fullpath, "%s/%s", searchinput, dir->d_name);

		int count = readlink(fullpath, buf, sizeof(buf) - 1);	
	
		if( (lstat(fullpath, &tr)) != 0 ){
			fprintf(stderr, "Error with stat: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	
		if( (stat(fullpath, &filestat)) != 0 ){
			fprintf(stderr, "Error with stat: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	
		/*Is a FILE */
		if(dir->d_type == DT_REG){
			info(dir->d_name, fullpath, filestat, mtime, user);
		}
	
		/*Is a symbolic link */
		else if(dir->d_type == DT_LNK && target != NULL){
			
			if(count >= 0){
				buf[count] = '\0';
				printf("\n -> .. %s\n", buf);
			}
		}

		/* Is a DIR */
		else if((dir->d_type == DT_DIR && (!traverse)) || ((tr.st_dev == filestat.st_dev))){
			//printf("%s\n ", fullpath);
			finder(fullpath, mtime, user, target, traverse);
		}
		
		/* Is unknown */
		else if(dir->d_type == DT_UNKNOWN){
			fprintf(stderr, "Error: unknown dir type: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

	}

	if( closedir(dirp) != 0){
		fprintf(stderr, "Error closing: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return 0;

}

int main(int argc, char *argv[]){

	int opt;
	int mtime = 0;
	char* user = NULL;
	char* target = NULL;
	int traverse = 0;

// Handle the input flags
	while((opt = getopt(argc,argv,"u:m:xl:")) != -1){
	printf("%s      ", optarg);
		switch(opt){
	
		case 'u':
			if(optarg == NULL){
				fprintf(stderr, "Did not specify user");
				exit(EXIT_FAILURE);
			}
			strcpy(user, optarg);
			break;

		case 'm':
			if(optarg == NULL){
				fprintf(stderr, "Did not specify mtime");
				exit(EXIT_FAILURE);
			}
			mtime = atoi(optarg);
			break;	
		
		case 'x':
			traverse = 1;
			if(traverse == 1){
				fprintf(stderr, "note: not crossing mount point\n\n");
			}		
			break;

		case 'l':
			if(optarg == NULL){
				fprintf(stderr, "Did not specify target");
				exit(EXIT_FAILURE);
			}
			strcpy(target, optarg);
			break;
		default:
			fprintf(stderr, "Usage: %s [-u user] [- m mtime] \n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if(argc == optind){
		fprintf(stderr, "No input specified");
		exit(EXIT_FAILURE);
	}

	char *searchinput = argv[optind];

	finder(searchinput, mtime, user, target, traverse);

	return 0;
}
