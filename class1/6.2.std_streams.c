#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[]) {
	
	int FILE = open(argv[1], O_RDWR | S_IREAD | O_APPEND);
	char readbuff[2];
	
	if (FILE == -1)
    	printf("failed to open file");
	
	while (true) {
		
		int result = read(FILE, readbuff, 1);
		
		if (result == -1 || result == 0) break;
		
		printf("%s", readbuff);
		
	}
	
	return 0;
}