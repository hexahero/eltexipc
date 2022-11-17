#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFSIZE 1024

int 		sockfd, newsockfd;
int 		portno;
int 		pid;
socklen_t 	clilen;

struct sockaddr_in serv_addr, cli_addr;

FILE* file;

void error(const char* msg) {
	perror(msg);
	exit(1);
}

bool initialize_socket(int argc, char* argv[]) {
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0) {
		perror("socket() error ");
		return false;
	}
	
	bzero((char*) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("bind() error ");
		return false;
	}
	
	return true;
}

bool open_file() {
	
	file = fopen("received", "wb");
	
	if (file == NULL) {
		error("fopen() error ");
		return false;
	}
	
	return true;
}

void wait_for_connection() {
	
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	
}

void receive_file() {
	
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	
	if (newsockfd < 0) 
		perror("accept() error");
	
	open_file();
	
	int bytesReceived;
	char buff[BUFFSIZE];
	
	while (true) {
		
		bzero(buff, BUFFSIZE);
		
		bytesReceived = read(newsockfd, buff, sizeof(buff));
		
		if (bytesReceived < 0)
			perror("read() error ");
		
		fwrite(buff, sizeof(char), strlen(buff), file);
		
		if (strlen(buff) < BUFFSIZE)
			break;
		
	}
	
	fclose(file);
	
	printf("file received\n");
	
}

int main(int argc, char* argv[]) {

	if (argc != 2)  {
		printf("Incorrect. Arguments are: <port>\n");
		exit(1);
	}
	
	if (!initialize_socket(argc, argv))
		exit(1);
	
	wait_for_connection();
		
	receive_file();
	
	close(sockfd);
	
	return 0;
}