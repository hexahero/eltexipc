#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFSIZE 1024

struct 	sockaddr_in serv_addr;
struct 	hostent* server;
int 	my_sock, portno, n;
char 	buff[BUFFSIZE];

FILE* file;

void error(const char* msg) {
	perror(msg);
	exit(1);
}

bool initialize_socket(int argc, char* argv[]) {
	
	portno = atoi(argv[2]);
	
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (my_sock < 0) {
		error("socket() error ");
		return false;
	}
	
	server = gethostbyname(argv[1]);
	
	if (server == NULL) {
		perror("gethostbyname() error ");
		return false;
	}
	
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	
	serv_addr.sin_port = htons(portno);
	
	return true;
}

bool initiate_connection() {
	
	if (connect(my_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect() error ");
		return false;
	}
	
	return true;
}

bool open_file(int argc, char* argv[]) {
	
	file = fopen(argv[3], "rb");
	
	if (file == NULL) {
		perror("fopen() error ");
		return false;
	}
	
	return true;
}

int main(int argc, char* argv[]) {
	
	if (argc != 4)  {
		printf("Incorrect. Arguments are: <addr> <port> <filename>\n");
		exit(1);
	}
	
	if (!initialize_socket(argc, argv))
		exit(1);
	
	if (!initiate_connection())
		exit(1);
	
	if (!open_file(argc, argv))
		exit(1);
	
	while (true) {
		
		bzero(buff, BUFFSIZE);
		
		if (fread(buff, sizeof(char), BUFFSIZE, file) < 0) {
			perror("fread() error");
			return 1;
		}
		
		send(my_sock, buff, strlen(buff), 0);
		
		if (strlen(buff) < BUFFSIZE)
			break;
		
	}
	
	printf("file sent\n");
	
	fclose(file);
	close(my_sock);
	
	return 0;
}