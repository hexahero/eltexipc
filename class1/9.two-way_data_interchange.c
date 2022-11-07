#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <fcntl.h>
#include <time.h>

//#define DEBUGINFO

#define MINRAND 0x3E8
#define MAXRAND 0x2710

int 	fileDescriptorParent[2];
int 	fileDescriptorChild[2];

char* 	destFileName;
int 	randomNumsToGenerate;

unsigned int 	buffsize;
int* 			buffWrite;
int* 			buffRead;

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
	
	// Close descriptors we are not going to use
	close(fileDescriptorChild[0]);
	close(fileDescriptorParent[1]);
	
	// Fill buffer with random numbers
	for (int i = 0; i < randomNumsToGenerate; i++)
		buffWrite[i] = generate_random_number(MINRAND, MAXRAND);
	
	// Send buffer to the parent branch
	write(fileDescriptorChild[1], buffWrite, buffsize);
	
	// Close write descriptor
	close(fileDescriptorChild[1]);
	
	// Wait for data from the parent branch
	read(fileDescriptorParent[0], buffRead, buffsize);
	
	// Print received data
	for (int i = 0; i < randomNumsToGenerate; i++)
		printf("multiplied by two: %d\n", buffRead[i]);
	
	// Close read descriptor
	close(fileDescriptorParent[0]);
	
}

void parent_subroutine() {
	
	// Close descriptors we are not going to use
    close(fileDescriptorChild[1]);
    close(fileDescriptorParent[0]);
	
	// Wait for data from the child branch
    read(fileDescriptorChild[0], buffRead, buffsize);
	
	// Open file and check if it was successful
	int FILE = open(destFileName, O_CREAT | S_IWUSR | O_WRONLY);
	
	if(FILE < 0) {
		
        perror("file open failed ");
        exit(EXIT_FAILURE);
		
    }
	
	// Print received data, write it into the open file, and multiply by two
	// to send back to the child branch
	for(int i = 0; i < randomNumsToGenerate; i++) {
		
        printf("%d\n", buffRead[i]);
		
		char tempbuff[5];
		sprintf(tempbuff, "%d\n", buffRead[i]);
		
		write(FILE, tempbuff, sizeof(tempbuff));
		
		buffRead[i] = buffRead[i] * 2;
		
    }
	
	// Send data back to the child branch
	write(fileDescriptorParent[1], buffRead, buffsize);
	
	// Close all the open descriptors
	close(FILE);
    close(fileDescriptorChild[0]);
	close(fileDescriptorParent[1]);
	
}

bool open_pipe(int* fileDescriptor) {
	
	if (pipe(fileDescriptor)) {
		
        perror("pipe open failed");
		
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
	
	if(!open_pipe(fileDescriptorParent))
		exit(EXIT_FAILURE);
	
	if(!open_pipe(fileDescriptorChild))
		exit(EXIT_FAILURE);
	
    if(!fork_execution(fork()))
		exit(EXIT_FAILURE);
	
	cleanup();
    
    exit(EXIT_SUCCESS);
}