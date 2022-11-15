#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/sem.h>

#include <fcntl.h>
#include <time.h>

//#define DEBUGINFO

#define MINRAND 0x3E8
#define MAXRAND 0x2710

union semun {

	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;

};

key_t 	key;
int 	semid;

union 	semun arg;
struct 	sembuf lock_res = { 0, -1, 0 };
struct 	sembuf rel_res 	= { 0, 	1, 0 };

int 	fileDescriptor[2];
char* 	destFileName;
int 	randomNumsToGenerate;

unsigned int buffsize;

int* buffWrite;
int* buffRead;

bool initialize_semaphore(char* argv[]) {
	
	if ((key = ftok(argv[0], 's')) == -1) {
		
		printf("ftok() error\n");
		
		return false;
	}
		
	if ((semid = semget(key, 1, 0666 | IPC_CREAT)) == -1) {
		
		printf("semget() error\n");
		
		return false;
	}
	
	arg.val = 1;
	semctl(semid, 0, SETVAL, arg);
	
	return true;
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

}

void cleanup() {
	
	free(buffWrite);
	free(buffRead);
	
	semctl(semid, 0, IPC_RMID);
	
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
		if (semop(semid, &lock_res, 1) == -1)
			perror("semop:lock_res");
		
		char sendbuff[1024];
		
		read(FILE, sendbuff, sizeof(sendbuff));
		
		write(fileDescriptor[1], sendbuff, sizeof(sendbuff));
		
		// Unlock
		semop(semid, &rel_res, 1);
		
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
		if (semop(semid, &lock_res, 1) == -1)
			perror("semop:lock_res");
		
		// Do stuff
		sleep(1);
		
		int newRandNum = generate_random_number(MINRAND, MAXRAND);
		
		char sendbuff[5];
		char recvbuff[1024];
		
		sprintf(sendbuff, "%d\n", newRandNum);
		
		write(FILE, sendbuff, sizeof(sendbuff));
		
		memset(sendbuff, 0, sizeof(sendbuff));
		
		// Unlock
		semop(semid, &rel_res, 1);
		
		// Read
		read(fileDescriptor[0], recvbuff, sizeof(recvbuff));
		printf("num read by the child: %s", recvbuff);
		
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
	
	if (!initialize_semaphore(argv))
		exit(EXIT_FAILURE);
	
    if(!fork_execution(fork()))
		exit(EXIT_FAILURE);
	
	cleanup();
    
    exit(EXIT_SUCCESS);
}