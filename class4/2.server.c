#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int 	sockfd;
int 	clilen, n;
char 	sendline[1000], recvline[1000];
struct 	sockaddr_in servaddr, cliaddr;

pthread_t receiverThread;

void initialize_socket(int argc, char* argv[]) {
	
	if (argc != 2)
		exit(1);
	
	int port = atoi(argv[1]);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror(NULL);
		exit(1);
	}
	
	if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		perror(NULL);
		close(sockfd);
		exit(1);
	}
	
}

void* receive() {
	
	while (true) {
		
		memset(recvline, 0, 1000);
		
		if ((n = recvfrom(sockfd, recvline, 999, 0,
			(struct sockaddr *) &cliaddr, &clilen)) < 0) {
			perror(NULL);
			close(sockfd);
			exit(1);
		}
		
		printf("%s", recvline);
	
	}
	
}

int main(int argc, char* argv[]) {
	
	initialize_socket(argc, argv);
	
	pthread_create(&receiverThread, NULL, &receive, NULL);
	pthread_detach(receiverThread);
	
	clilen = sizeof(cliaddr);
	
	while (true) {
		
		fgets(sendline, 1000, stdin);
		
		if (sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *) &cliaddr, clilen) < 0) {
			perror(NULL);
			close(sockfd);
			exit(1);
		}
		
	}
	
	return 0;
}