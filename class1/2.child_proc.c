#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void exitp(void) {
	
    printf("%d exited\n", getpid());
}

int main(int argc, char* argv[]) {
	
	switch(fork()) {
		
		case -1: {
			
			perror("fork");
			exit(EXIT_FAILURE);
			
			break;
		}
		
		case 0: {
			
			for(int i = 1; i < argc; ++i)
				printf("child proc\t%d:\t%s\n", getpid(), argv[i]);
			
			break;
		}
		
		default: {
			
			for(int i = 1; i < argc; ++i)
				printf("parent proc\t%d:\t%s\n", getpid(), argv[i]);
			
		}
		
	}
	
   	if(atexit(exitp)) {
	
		printf("exit error");
		return 1; 
		
	}
	
}