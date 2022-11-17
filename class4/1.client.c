#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int 	sockfd; // Дескриптор сокета
int 	n, len; // Переменные для различных длин и количества символов
char 	sendline[1000], recvline[1000]; // Массивы для отсылаемой и принятой строки
struct 	sockaddr_in servaddr, cliaddr; 	// Структуры для адресов сервера и клиента

void initialize_socket(int argc, char* argv[]) {
	
	// Сначала проверяем наличие второго аргумента в
	// командной строке. При его отсутствии ругаемся и прекращаем работу
	if (argc != 3) {
		printf("Usage: <IP address> <port>\n");
		exit(1);
	}
	
	int port = atoi(argv[2]);
	
	// Создаем UDP сокет
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror(NULL); // Печатаем сообщение об ошибке
		exit(1);
	}
	
	// Заполняем структуру для адреса клиента
	bzero(&cliaddr, sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(0);
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// Настраиваем адрес сокета
	if (bind(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0) {
		perror(NULL);
		close(sockfd); // По окончании работы закрываем дескриптор сокета
		exit(1);
	}
	
	// Заполняем структуру для адреса сервера
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	
	if (inet_aton(argv[1], &servaddr.sin_addr) == 0){
		printf("Invalid IP address\n");
		close(sockfd); // По окончании работы закрываем дескриптор сокета
		exit(1);
	}
	
}

int main(int argc, char* argv[]) {
	
	// Инициализируем сокет
	initialize_socket(argc, argv);
	
	// Вводим строку, которую отошлем серверу
	printf("String => ");
	fgets(sendline, 1000, stdin);
	
	// Отсылаем датаграмму
	if (sendto(sockfd, sendline, strlen(sendline)+1, 0, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		perror(NULL);
		close(sockfd);
		exit(1);
	}
	
	// Ожидаем ответа и читаем его. Максимальная
	// допустимая длина датаграммы – 1000 символов,
	// адрес отправителя нам не нужен
	if ((n = recvfrom(sockfd, recvline, 1000, 0,
		(struct sockaddr *) NULL, NULL)) < 0) {
		perror(NULL);
		close(sockfd);
		exit(1);
	}
	
	// Печатаем пришедший ответ и закрываем сокет
	printf("%s\n", recvline);
	close(sockfd);
	
	return 0;
}