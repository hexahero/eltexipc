#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 128

int main(int argc, char *argv[]) {
	
	int FILE = open(argv[1], O_RDWR | S_IWRITE | O_APPEND);
	char inputbuff[INPUT_BUFFER_SIZE];
	
	if (FILE == -1)
    	printf("failed to open file");
	
	printf(">");
	
	for (int i = 0; i < INPUT_BUFFER_SIZE; i++) {
		
		scanf("%c", &inputbuff[i]);
		
		if (inputbuff[i] == '\n') {
			
			inputbuff[i + 1] = '\0';
			break;
		}
		
	}
	
	write(FILE, inputbuff, strlen(inputbuff));
	
	return 0;
}