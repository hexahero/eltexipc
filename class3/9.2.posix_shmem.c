#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

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

#define SEMNAME_PROD "semnameProd"
#define SEMNAME_CONS "semnameCons"

#define SHMNAME "shmname"

sem_t* semaphoreProd;
sem_t* semaphoreCons;

unsigned int 	pageSize;	   
char* 			shmemBuff;

int* shmemBuffPtrInt;
int numSetsProcessed;

bool initialize_shared_memory() {
	
	pageSize = getpagesize();
	
	int fd = shm_open(SHMNAME, O_RDWR, S_IRUSR | S_IWUSR);
	
	if (fd == -1) {
		perror("shm_open() error");
		return false;
	}
	
    shmemBuff = mmap(
		NULL, 
		sizeof(pageSize),
		PROT_READ | PROT_WRITE, 
		MAP_SHARED, 
		fd, 
		0
	);
	
    if (shmemBuff == MAP_FAILED) {
        perror("mmap() error");
		return false;
	}
	
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
	
	shmemBuffPtrInt[pageSize - 1] = 127;
	printf("num sets processed: %d\n", numSetsProcessed);
	
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

int main() {
	
	if (!initialize_shared_memory())
		exit(EXIT_FAILURE);
	
	if (!initialize_semaphore())
		exit(EXIT_FAILURE);
	
	signal(SIGINT, sigint_response);
	signal(SIGTERM, sigint_response);
	atexit(exit_handler);
	
	shmemBuffPtrInt = (int*)shmemBuff;
	numSetsProcessed = 0;
	
	int minNum = INT_MAX;
	int maxNum = 0;
	
	while (true) {
		
		// Lock
		sem_wait(semaphoreProd);
		
		// Do stuff
		unsigned int ctr = 0;
		for (ctr; ctr != pageSize - 1; ctr++) {
			
			if (shmemBuffPtrInt[ctr] == 0)
				break;
			
			if (shmemBuffPtrInt[ctr] < minNum)
				minNum = shmemBuffPtrInt[ctr];
			
			if (shmemBuffPtrInt[ctr] > maxNum)
				maxNum = shmemBuffPtrInt[ctr];
			
		}
		
		memset(shmemBuff, 0, ctr * sizeof(int));
		
		shmemBuffPtrInt[0] = minNum;
		shmemBuffPtrInt[1] = maxNum;
		
		printf("%d\n%d\n\n", minNum, maxNum);
		
		minNum = INT_MAX;
		maxNum = 0;
		
		numSetsProcessed++;
		
		// Unlock
		sem_post(semaphoreCons);
		
	}
	
	exit(EXIT_SUCCESS);
}