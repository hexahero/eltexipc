#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <sys/msg.h>
#include <time.h>

#define MAXMSG 	32
#define MSGTYPE 0

typedef struct Message {
	
    long int messageType;
    char message[BUFSIZ];
	
} Message;

int main() {
	
    bool shutdown = false;
	
	struct Message msg;
	int messageQueueID;
   
	messageQueueID = msgget(ftok("file.c", '1'), 0666 | IPC_CREAT);
	
	if (messageQueueID == -1) {
		
		printf("message queue creation failure\n");
		exit(EXIT_FAILURE);
		
	}
	
	while (!shutdown) {
		
		msgrcv(messageQueueID, (void*)&msg, BUFSIZ, MSGTYPE, 0);
		printf("%s", msg.message);

		if (strncmp(msg.message, "255", 3) == 0)
			shutdown = true;
		
	}
	
	/* while (!shutdown) {
		
		msgrcv(messageQueueID, (void*)&msg, BUFSIZ, MSGTYPE, 0);
		printf("received message: %s", msg.message);

		if (strncmp(msg.message, "quit", 4) == 0)
			shutdown = true;
	
	} */
	
	msgctl(messageQueueID, IPC_RMID, NULL);
	exit(EXIT_SUCCESS);
}