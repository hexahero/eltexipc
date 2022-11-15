#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/sem.h>

#include <time.h>
#include <fcntl.h>
#include <errno.h>

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

const char* pipeName = "namedPipe";
mode_t		pipeMode = 0777;
int 		pipeHandle;

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

bool open_named_pipe() {
	
	if (mkfifo(pipeName, pipeMode) == -1) {
	
        if(errno != EEXIST) {
			
			perror("failed to create FIFO");
			
			return false;
		}
        
    }
	
	return true;
}

int main(int argc, char* argv[]) {
	
	if (!initialize_semaphore(argv))
		exit(EXIT_FAILURE);
	
	if (!open_named_pipe())
		exit(EXIT_FAILURE);
	
	pipeHandle = open(pipeName, O_WRONLY);
	
	while (true) {
		
		// Lock
		semid = semget(key, 1, 0);
		if (semop(semid, &lock_res, 1) == -1)
			perror("semop:lock_res");
		
		// Do stuff
		int printNum = atoi(argv[2]);	// Using a fixed value to visually
										// distinguish data read by the consumer.
		if (write(pipeHandle, &printNum, sizeof(printNum)) == -1) {
			
			perror("FIFO write failed ");
			exit(EXIT_FAILURE);
			
		}
		
		sleep(atoi(argv[1]));
		
		// Unlock
		semop(semid, &rel_res, 1);
		
	}
	
	semctl(semid, 0, IPC_RMID);
	close(pipeHandle);
	
	exit(EXIT_SUCCESS);
}