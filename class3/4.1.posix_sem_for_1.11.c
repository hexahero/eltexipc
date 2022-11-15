#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <signal.h>
#include <semaphore.h>

#include <time.h>
#include <fcntl.h>
#include <errno.h>

#define SEMNAME "semname"

const char* pipeName = "namedPipe";
mode_t		pipeMode = 0777;
int 		pipeHandle;

sem_t* semaphore;

bool initialize_semaphore() {
	
	if ((semaphore = sem_open(SEMNAME, O_CREAT | O_EXCL, 0664, 1)) == SEM_FAILED) {
		
		if((semaphore = sem_open(SEMNAME, 0)) == SEM_FAILED) {
			
			perror("sem_open() error ");
		
			return false;
		}
		
	}
	
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

void exit_handler() {
	
	sem_unlink(SEMNAME);
	sem_close(semaphore);
	
	close(pipeHandle);
	
}

void sigint_response(int sigCode) {
	
	exit_handler();
	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
	
	if (!initialize_semaphore())
		exit(EXIT_FAILURE);
	
	if (!open_named_pipe())
		exit(EXIT_FAILURE);
	
	pipeHandle = open(pipeName, O_WRONLY);
	
	signal(SIGINT, sigint_response);
	signal(SIGTERM, sigint_response);
	atexit(exit_handler);
	
	while (true) {
		
		// Lock
		sem_wait(semaphore);
		
		int val;
		sem_getvalue(semaphore, &val);
		printf("%d %d\n", getpid(), val);
		
		// Do stuff
		int printNum = atoi(argv[2]);	// Using a fixed value to visually
										// distinguish data read by the consumer.
		if (write(pipeHandle, &printNum, sizeof(printNum)) == -1) {
			
			perror("FIFO write failed ");
			exit(EXIT_FAILURE);
			
		}
		
		sleep(atoi(argv[1]));
		
		// Unlock
		sem_post(semaphore);
		sem_getvalue(semaphore, &val);
		printf("%d %d\n", getpid(), val);
		
	}
	
	exit(EXIT_SUCCESS);
}