#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#define MINRAND 0x3E8
#define MAXRAND 0x2710

const char* pipeName = "namedPipe";
mode_t		pipeMode = 0777;
int 		pipeHandle;

int generate_random_number(unsigned int minval, unsigned int maxval) {
	
	return (rand() % (maxval - minval + 1)) + minval;
}

bool open_named_pipe(const char* pipeName, mode_t pipeMode) {
	
	if (mkfifo(pipeName, pipeMode) == -1) {
	
        if(errno != EEXIST) {
			
			perror("failed to create FIFO");
			
			return false;
		}
        
    }
	
	return true;
}

int main() {
	
	srand(time(NULL) * getpid());
	
	if (!open_named_pipe(pipeName, pipeMode))
		exit(EXIT_FAILURE);
	
	pipeHandle = open(pipeName, O_WRONLY);
	
	while (true) {
		
		int randNum = generate_random_number(MINRAND, MAXRAND);
		
		if (write(pipeHandle, &randNum, sizeof(randNum)) == -1) {
			
			perror("FIFO write failed ");
			exit(EXIT_FAILURE);
			
		}
		
	}
	
	close(pipeHandle);
	
	exit(EXIT_SUCCESS);
}