#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

bool shouldWriteToFile = false;

void sigterm_response(int sigCode) {
	
	shouldWriteToFile = false;
	printf("file write stopped\n");
	
	signal(SIGTERM, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	
}

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
	else {
		
		signal(SIGTERM, sigterm_response);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		
		shouldWriteToFile = true;
		
	}

    while(shouldWriteToFile) {
	
		static int ctr = 1;
		
		char tempbuff[32] = { 0 };
		strcat(strcat(tempbuff, itoa(ctr, 10)), "\n");
		
        write(FILE, tempbuff, strlen(tempbuff));
		
		ctr++;
		
        sleep(1);
    }
	
    close(FILE);

	sleep(20);
	
    exit(EXIT_SUCCESS);
}