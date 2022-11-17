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
int 	n, len;
char 	sendline[1000], recvline[1000];
struct 	sockaddr_in servaddr, cliaddr;

pthread_t receiverThread;

void initialize_socket(int argc, char* argv[]) {
	
	if (argc != 2)
		exit(1);
	
	int port = atoi(argv[1]);
	
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror(NULL);
		exit(1);
	}
	
	bzero(&cliaddr, sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(0);
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0) {
		perror(NULL);
		close(sockfd);
		exit(1);
	}
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	
	if (inet_aton("127.0.0.1", &servaddr.sin_addr) == 0){
		printf("Invalid IP address\n");
		close(sockfd);
		exit(1);
	}
	
}

void* receive() {
	
	while (true) {
		
		memset(recvline, 0, 1000);
		
		if ((n = recvfrom(sockfd, recvline, 1000, 0, (struct sockaddr *) NULL, NULL)) < 0) {
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
	
	while (true) {
		
		fgets(sendline, 1000, stdin);
		
		if (sendto(sockfd, sendline, strlen(sendline) + 1, 0, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
			perror(NULL);
			close(sockfd);
			exit(1);
		}
		
	}
	
	close(sockfd);
	
	return 0;
}