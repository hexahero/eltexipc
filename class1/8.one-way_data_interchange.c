#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <fcntl.h>
#include <time.h>

//#define DEBUGINFO

#define MINRAND 0x3E8
#define MAXRAND 0x2710

int 	fileDescriptor[2];
char* 	destFileName;
int 	randomNumsToGenerate;

unsigned int buffsize;

int* buffWrite;
int* buffRead;

int generate_random_number(unsigned int minval, unsigned int maxval) {
	
	return (rand() % (maxval - minval + 1)) + minval;
}

void setup(char *argv[]) {
	
	destFileName = argv[1];
	randomNumsToGenerate = atoi(argv[2]);
	
	buffsize = randomNumsToGenerate * sizeof(int);
	
	buffWrite = (int*)malloc(buffsize);
	buffRead = (int*)malloc(buffsize);
	
	#ifdef DEBUGINFO
		printf("%s\n", destFileName);
		printf("%d\n", randomNumsToGenerate);
		printf("%d\n", buffsize);
	#endif
	
	srand(time(NULL) * getpid());

}

void cleanup() {
	
	free(buffWrite);
	free(buffRead);
	
}

void child_subroutine() {
	
	close(fileDescriptor[0]);
		
	for (int i = 0; i < randomNumsToGenerate; i++)
		buffWrite[i] = generate_random_number(MINRAND, MAXRAND);
	
	write(fileDescriptor[1], buffWrite, buffsize);
	close(fileDescriptor[1]);
	
}

void parent_subroutine() {
	
    close(fileDescriptor[1]);
    read(fileDescriptor[0], buffRead, buffsize);
	
	int FILE = open(destFileName, O_CREAT | S_IWUSR | O_WRONLY);
	
	if(FILE < 0) {
		
        perror("file open failed ");
        exit(EXIT_FAILURE);
		
    }
	
	for(int i = 0; i < randomNumsToGenerate; i++) {
		
        printf("%d\n", buffRead[i]);
		
		char tempbuff[5];
		sprintf(tempbuff, "%d\n", buffRead[i]);
		
		write(FILE, tempbuff, sizeof(tempbuff));
		
    }
	
	close(FILE);
	
    close(fileDescriptor[0]);
	
}

bool open_pipe() {
	
	if (pipe(fileDescriptor)) {
		
        printf("pipe open failed");
		
        return false;
    }
	
	return true;
}

bool fork_execution(int forkID) {
	
	if (forkID == -1)
		return false;
	else if (forkID == 0)
		child_subroutine();
    else
		parent_subroutine();
	
	return true;
}

int main(int argc, char *argv[]) {
	
	setup(argv);
	
	if(!open_pipe())
		exit(EXIT_FAILURE);
	
    if(!fork_execution(fork()))
		exit(EXIT_FAILURE);
	
	cleanup();
    
    exit(EXIT_SUCCESS);
}