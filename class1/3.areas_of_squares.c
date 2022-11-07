#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int square(int valToSquare) {
	
	return valToSquare * valToSquare;
}

int main(int argc, char* argv[]) {
	
	switch(fork()) {
		
		case -1: {
			
			exit(EXIT_FAILURE);
			
			break;
		}
		
		case 0: {
			
			for(int i = (argc / 2) + 1; i < argc; ++i)
				printf("proc (%d) area of the square:\t%d\n", getpid(), square(atoi(argv[i])));
			
			break;
		}
		
		default: {
			
			for(int i = 1; i <= argc / 2; ++i)
				printf("proc (%d) area of the square:\t%d\n", getpid(), square(atoi(argv[i])));
				
			
		}
		
	}
	
    exit(EXIT_SUCCESS); 
}