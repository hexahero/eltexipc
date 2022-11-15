#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <fcntl.h>

int main() {
	
    int pipeHandle = open("namedPipe", O_RDONLY);
	
    while(true) {
		
        int acceptBuffer;
		
        if (read(pipeHandle, &acceptBuffer, sizeof(acceptBuffer)) == -1) {
			
            perror("FIFO read failed ");
			
            exit(EXIT_FAILURE);
        }
		
        printf("%d\n", acceptBuffer);
		
    }
	
    close(pipeHandle);
	
    exit(EXIT_SUCCESS);
}