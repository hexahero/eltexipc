#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

#define MAXMSG 	32

#define MSGTYPE0 0
#define MSGTYPE1 1

#define QUEUEPERM 0666

typedef struct Message {
	
    long int messageType;
    char message[MAXMSG];
	
} Message;

bool shutdown = false;
	
struct Message msgsend;
struct Message msgrecv;
char msgbuffer[16] = { 0 };

int messageQueueSendID;
int messageQueueReceiveID;

void cleanup() {
	
	msgctl(messageQueueSendID, IPC_RMID, NULL);
	msgctl(messageQueueReceiveID, IPC_RMID, NULL);
	
}

void recv_loop() {
	
	while (!shutdown) {
		
		msgrcv(messageQueueReceiveID, (void*)&msgrecv, BUFSIZ, MSGTYPE0, 0);
		printf("received message: %s", msgrecv.message);
	
	}
	
}

void send_loop() {
	
	while (!shutdown) {
		
		printf(">");
		fgets(msgbuffer, sizeof(msgbuffer), stdin);
		
		msgsend.messageType = MSGTYPE1;
		stpcpy(msgsend.message, msgbuffer);
		
		if (msgsnd(messageQueueSendID, (void*)&msgsend, MAXMSG, IPC_NOWAIT) == -1) {
			
			printf("failed to send message\n");
			exit(EXIT_FAILURE);
			
		}
		
		if (strncmp(msgbuffer, "quit", 4) == 0)
			shutdown = true;
	
	}
	
}

bool fork_execution(int forkID) {
	
	if (forkID == -1) {
		cleanup();
		printf("fork() error\n");
		return false;
	}
	else if (forkID == 0)
		recv_loop();
	else
		send_loop();
		
	return true;
}

int main(int argc, char* argv[]) {
	
	int sendQueueKey;
	int recvQueueKey;
	
	printf("enter destination que key>");
	scanf("%d", &sendQueueKey);
	printf("enter receive que key>");
	scanf("%d", &recvQueueKey);
	
	messageQueueSendID = msgget(sendQueueKey,  QUEUEPERM | IPC_CREAT);
	messageQueueReceiveID = msgget(recvQueueKey, QUEUEPERM | IPC_CREAT);
	
	if (messageQueueSendID == -1 || messageQueueReceiveID == -1) {
		
		perror("failed to create message queue ");
		exit(EXIT_FAILURE);
		
	}
	
	if (!fork_execution(fork()))
		exit(EXIT_FAILURE);
		
	cleanup();
	exit(EXIT_SUCCESS);
}