#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int 	sockfd; 		// Дескриптор сокета
int 	clilen, n; 		// Переменные для различных длин и количества символов
char 	line[1000]; 	// Массив для принятой и отсылаемой строки
struct 	sockaddr_in servaddr, cliaddr; // Структуры для адресов сервера и клиента

void initialize_socket(int argc, char* argv[]) {
	
	// Заполняем структуру для адреса сервера
	if (argc != 2) {
		printf("Usage: <port>\n");
		exit(1);
	}
	
	int port = atoi(argv[1]);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// Создаем UDP сокет
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror(NULL); // Печатаем сообщение об ошибке
		exit(1);
	}
	
	// Настраиваем адрес сокета
	if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		perror(NULL);
		close(sockfd);
		exit(1);
	}
	
}

int main(int argc, char* argv[]) {
	
	// Инициализируем сокет
	initialize_socket(argc, argv);
	
	while (true) {
		
		// Основной цикл обслуживания
		clilen = sizeof(cliaddr);
		
		// Ожидаем прихода запроса от клиента и читаем его
		if ((n = recvfrom(sockfd, line, 999, 0,
			(struct sockaddr *) &cliaddr, &clilen)) < 0) {
			perror(NULL);
			close(sockfd);
			exit(1);
		}
		
		// Печатаем принятый текст на экране
		printf("%s\n", line);
		
		// Принятый текст отправляем обратно по адресу отправителя
		if (sendto(sockfd, line, strlen(line), 0,
			(struct sockaddr *) &cliaddr, clilen) < 0) {
			perror(NULL);
			close(sockfd);
			exit(1);
		} // Уходим ожидать новую датаграмму
		
	}
	
	return 0;
}