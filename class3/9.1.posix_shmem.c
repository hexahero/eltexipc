#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>
#include <semaphore.h>

#include <time.h>
#include <fcntl.h>
#include <errno.h>

#define MINRAND 0x3E8
#define MAXRAND 0x2710

#define SEMNAME_PROD "semnameProd"
#define SEMNAME_CONS "semnameCons"

#define SHMNAME "shmname"

sem_t* semaphoreProd;
sem_t* semaphoreCons;

unsigned int 	pageSize;	   
char* 			shmemBuff;

bool initialize_shared_memory() {
	
	shm_unlink(SHMNAME);
	
	pageSize = getpagesize();
	
	int fd = shm_open(SHMNAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	
	if (fd == -1) {
		perror("shm_open() error");
		return false;
	}

	ftruncate(fd, pageSize);
    shmemBuff = mmap(
		NULL, 
		sizeof(shmemBuff),
		PROT_READ | PROT_WRITE, 
		MAP_SHARED, 
		fd, 
		0
	);
	
    if (shmemBuff == MAP_FAILED) {
        perror("mmap() error");
		return false;
	}
	
	memset(shmemBuff, 0, pageSize);
	
	return true;
}

bool initialize_semaphore() {
	
	if (
		(semaphoreProd = sem_open(SEMNAME_PROD, O_CREAT | O_EXCL, 0664, 1)) == SEM_FAILED ||
		(semaphoreCons = sem_open(SEMNAME_CONS, O_CREAT | O_EXCL, 0664, 0)) == SEM_FAILED
	) {
		
		if (
			(semaphoreProd = sem_open(SEMNAME_PROD, 0)) == SEM_FAILED  ||
			(semaphoreCons = sem_open(SEMNAME_CONS, 0)) == SEM_FAILED
		) {
			perror("sem_open() error ");
			return false;
		}
		
	}
	
	return true;
}

void exit_handler() {
	
	sem_unlink(SEMNAME_PROD);
	sem_unlink(SEMNAME_CONS);
	sem_close(semaphoreProd);
	sem_close(semaphoreCons);
	
	shm_unlink(SHMNAME);
	munmap(shmemBuff, pageSize);
	
	exit(EXIT_SUCCESS);
}

void sigint_response(int sigCode) {
	
	exit_handler();
	exit(EXIT_SUCCESS);
}

int generate_random_number(unsigned int minval, unsigned int maxval) {
	
	return (rand() % (maxval - minval + 1)) + minval;
}

int main(int argc, char* argv[]) {
	
	if (!initialize_shared_memory())
		exit(EXIT_FAILURE);
	
	if (!initialize_semaphore())
		exit(EXIT_FAILURE);
	
	srand(time(NULL) * getpid());
	
	signal(SIGINT, sigint_response);
	signal(SIGTERM, sigint_response);
	atexit(exit_handler);
	
	int* shmemBuffPtrInt = (int*)shmemBuff;
	
	while (shmemBuffPtrInt[pageSize - 1] != 127) {
		
		// Lock
		sem_wait(semaphoreCons);
		
		printf(
			"\nvals found by the 2nd process\nminval: %d\nmaxval: %d\n\n",
			shmemBuffPtrInt[0],
			shmemBuffPtrInt[1]
		);
		
		// Do stuff
		int numsToGen = generate_random_number(2, 10);
		
		printf("==============================\nGenerated\n\n");
		
		for (int i = 0; i != numsToGen; i++) {
		
			int newNum = generate_random_number(MINRAND, MAXRAND);
			shmemBuffPtrInt[i] = newNum;
			printf("%d\n", newNum);
		
		}
		
		sleep(atoi(argv[1]));
		
		// Unlock
		sem_post(semaphoreProd);
		
	}
	
	exit(EXIT_SUCCESS);
}