#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

void sig_response(int sigCode) {
	
	static int sigintCtr = 0;
	
	if (sigCode == SIGINT) {
		
		printf("SIGINT signal received\n");
		sigintCtr++;
		
		if (sigintCtr == 3) {
			
			printf("terminated\n");
			exit(EXIT_SUCCESS);
		}
		
	}
	else if (sigCode == SIGQUIT)
		printf("SIGQUIT signal received\n");
	
}

char* itoa(int val, int base) {
	
	static char buf[32] = { 0 };
	
	int i = 30;
	
	for(; val && i; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i + 1];
}

int main() {
	
	char *filename = "file";
	int FILE;
	
	signal(SIGINT, sig_response);
	signal(SIGQUIT, sig_response);
	
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