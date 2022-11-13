#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>

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

bool accessAllowed;

void sigusr_response(int sigCode) {
	
	if (sigCode == SIGUSR1)
		accessAllowed = false;
	else if (sigCode == SIGUSR2)
		accessAllowed = true;
	
}

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
	
	signal(SIGUSR1, sigusr_response);
	signal(SIGUSR2, sigusr_response);

}

void cleanup() {
	
	free(buffWrite);
	free(buffRead);
	
}

void child_subroutine() {
	
	close(fileDescriptor[0]);
	
	int FILE = open(destFileName, O_RDONLY);
	
	while (true) {
		
		if (accessAllowed) {
			
			char sendbuff[1024];
			
			read(FILE, sendbuff, sizeof(sendbuff));
			
			write(fileDescriptor[1], sendbuff, sizeof(sendbuff));
			
		}
		
	}
	
	close(FILE);
	
}

void parent_subroutine() {
	
    close(fileDescriptor[1]);
	
	int FILE = open(destFileName, O_CREAT | O_WRONLY);
	
	if (FILE < 0) {
	
		perror("file open failed ");
		exit(EXIT_FAILURE);
	
	}
	
	while (true) {
		
		kill(0, SIGUSR1);
		
		sleep(1);
		
		int randNum = generate_random_number(MINRAND, MAXRAND);
		
		char sendbuff[5];
		char recvbuff[1024];
		
		sprintf(sendbuff, "%d\n", randNum);
		write(FILE, sendbuff, sizeof(sendbuff));
		
		memset(sendbuff, 0, sizeof(sendbuff));
		
		kill(0, SIGUSR2);
		
		read(fileDescriptor[0], recvbuff, sizeof(recvbuff));
		printf("num read by the child: %s\n", recvbuff);
		
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
	
	if (forkID == -1) {
		perror("fork() error ");
		return false;
	}
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