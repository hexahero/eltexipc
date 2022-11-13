#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

char* itoa(int val, int base){
	
	static char buf[32] = { 0 };
	
	int i = 30;
	
	for(; val && i; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i + 1];
}

int main() {
	
	char *filename = "file";
	int FILE;
	
    if((FILE = open(filename, O_WRONLY)) < 0) {
		
        perror("could not open file ");
        exit(EXIT_FAILURE);
		
    }

    while(true) {
		
		static int ctr = 1;
		
		char tempbuff[32] = { 0 };
		strcat(strcat(tempbuff, itoa(ctr, 10)), "\n");
		
        write(FILE, tempbuff, strlen(tempbuff));
		
		ctr++;
		
        sleep(1);
    }
	
    close(FILE);
    exit(EXIT_SUCCESS);
	
}