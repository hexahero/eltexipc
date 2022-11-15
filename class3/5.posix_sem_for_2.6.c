#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <signal.h>
#include <semaphore.h>

#include <fcntl.h>
#include <time.h>

//#define DEBUGINFO

#define SEMNAME "semname"
#define MINRAND 0x3E8
#define MAXRAND 0x2710

sem_t* semaphore;

int 	fileDescriptor[2];
char* 	destFileName;
int 	randomNumsToGenerate;

unsigned int buffsize;

int* buffWrite;
int* buffRead;

void cleanup();

bool initialize_semaphore() {
	
	if ((semaphore = sem_open(SEMNAME, O_CREAT | O_EXCL, 0664, 1)) == SEM_FAILED) {
		
		if((semaphore = sem_open(SEMNAME, 0)) == SEM_FAILED) {
			
			perror("sem_open() error ");
		
			return false;
		}
		
	}
	
	return true;
}

void exit_handler() {
	
	sem_unlink(SEMNAME);
	sem_close(semaphore);
	
}

void sigint_response(int sigCode) {
	
	exit_handler();
	exit(EXIT_SUCCESS);
	
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
	
	signal(SIGINT, sigint_response);
	signal(SIGTERM, sigint_response);
	atexit(exit_handler);

}

void cleanup() {
	
	free(buffWrite);
	free(buffRead);
	
}

void child_subroutine() {
	
	close(fileDescriptor[0]);
	int FILE = open(destFileName, O_RDONLY);
	
	if (FILE < 0) {
	
		perror("file open failed (child) ");
		exit(EXIT_FAILURE);
	
	}
		
	while (true) {
		
		// Lock
		sem_wait(semaphore);
		
		int val;
		sem_getvalue(semaphore, &val);
		printf("%d %d\n", getpid(), val);
		
		char sendbuff[1024];
		
		read(FILE, sendbuff, sizeof(sendbuff));
		
		write(fileDescriptor[1], sendbuff, sizeof(sendbuff));
		
		// Unlock
		sem_post(semaphore);
		sem_getvalue(semaphore, &val);
		printf("%d %d\n", getpid(), val);
		
	}
	
	close(FILE);
	
}

void parent_subroutine() {
	
    close(fileDescriptor[1]);
	int FILE = open(destFileName, O_CREAT | O_WRONLY);
		
	if (FILE < 0) {
	
		perror("file open failed (parent) ");
		exit(EXIT_FAILURE);
	
	}
	
	while (true) {
		
		// Lock
		sem_wait(semaphore);
		
		int val;
		sem_getvalue(semaphore, &val);
		printf("%d %d\n", getpid(), val);
		
		// Do stuff
		sleep(1);
		
		int newRandNum = generate_random_number(MINRAND, MAXRAND);
		
		char sendbuff[5];
		char recvbuff[1024];
		
		sprintf(sendbuff, "%d\n", newRandNum);
		
		write(FILE, sendbuff, sizeof(sendbuff));
		
		memset(sendbuff, 0, sizeof(sendbuff));
		
		// Unlock
		sem_post(semaphore);
		sem_getvalue(semaphore, &val);
		printf("%d %d\n", getpid(), val);
		
		// Read
		read(fileDescriptor[0], recvbuff, sizeof(recvbuff));
		printf("num read by the child: %s", recvbuff);
		
	}
	
	close(FILE);
    close(fileDescriptor[0]);
	
}

bool open_pipe() {
	
	if (pipe(fileDescriptor)) {
		
        perror("pipe open failed");
		
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
	
	if (!initialize_semaphore())
		exit(EXIT_FAILURE);
	
    if(!fork_execution(fork()))
		exit(EXIT_FAILURE);
    
    exit(EXIT_SUCCESS);
}