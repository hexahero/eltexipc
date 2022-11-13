#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

#define MAXMSG 	32
#define MSGTYPE 1

#define MINRAND 0xE6
#define MAXRAND 0x100

typedef struct Message {
	
    long int messageType;
    char message[MAXMSG];
	
} Message;

int generate_random_number(unsigned int minval, unsigned int maxval) {
	
	srand(time(NULL));
	
	return (rand() % (maxval - minval + 1)) + minval;
}

char* itoa(int val, int base) {
	
	static char buf[32] = { 0 };
	
	int i = 30;
	
	for(; val && i; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i + 1];
}

int main() {
	
	bool shutdown = false;
	
	struct Message msg;
	int messageQueueID;
	char msgbuffer[16] = { 0 };
   
	messageQueueID = msgget(ftok("file.c", '1'), 0666 | IPC_CREAT);
	
	if (messageQueueID == -1) {
		
		printf("message queue creation failure\n");
		exit(EXIT_FAILURE);
		
	}
	
	while (!shutdown) {
		
		int randomNum = generate_random_number(MINRAND, MAXRAND);
		
		strcat(strcat(msgbuffer, itoa(randomNum, 10)), "\n");
		stpcpy(msg.message, msgbuffer);
		
		msg.messageType = MSGTYPE;
		
		if (msgsnd(messageQueueID, (void*)&msg, MAXMSG, 0) == -1) {
			
			printf("failed to send message\n");
			exit(EXIT_FAILURE); 
			
		}
		
		sleep(1);
	
	}
	
	/* while (!shutdown) {
		
		printf(">");
		fgets(msgbuffer, sizeof(msgbuffer), stdin);
		
		msg.messageType = MSGTYPE;
		stpcpy(msg.message, msgbuffer);
		
		if (msgsnd(messageQueueID, (void*)&msg, MAXMSG, 0) == -1) {
			
			printf("failed to send message\n");
			exit(EXIT_FAILURE); 
			
		}
		
		if (strncmp(msgbuffer, "quit", 4) == 0)
			shutdown = true;
	
	} */
	
	msgctl(messageQueueID, IPC_RMID, NULL);
	exit(EXIT_SUCCESS);
}