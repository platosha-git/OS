#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>

#include "errors.h"

const int BUFSIZE = 24;
static char* shmbuf = NULL;
const char ALPHABET[] = "abcdefghijklmnopqrstuvwxyz";
const int AlphLen = strlen(ALPHABET);

static char* idxProd = 0;
static char* idxCons = 0;

static bool wasSignal = false;
static bool isEndConsume = false;


enum Semaphores
{
	BUFF_FULL,
	BUFF_EMPTY,
	BIN_SEM
};

struct sembuf prodP[2] = {{BUFF_EMPTY, -1, 0}, {BIN_SEM, -1, 0}};
struct sembuf prodV[2] = {{BIN_SEM, 1, 0}, {BUFF_FULL, 1, 0}};

struct sembuf consP[2] = {{BUFF_FULL, -1, 0}, {BIN_SEM, -1, 0}};
struct sembuf consV[2] = {{BIN_SEM, 1, 0}, {BUFF_EMPTY, 1, 0}};


const int perms = S_IRWXU | S_IRWXG | S_IRWXO;


const int NumProd = 3;
const int NumCons = 3;


void producer(const int semfd, const int id);
void consumer(const int semfd, const int id);

void errorCheck(int fd, int err, char* msg, int exCode);
void catchSignal(int signalNum);

int main()
{
	//память//
	int size = (BUFSIZE + 2) * sizeof(char);
	int shmfd = shmget(IPC_PRIVATE, size, IPC_CREAT | perms);
	errorCheck(shmfd, -1, "shmget", SHMGET_ERR);

	idxProd = (char*) shmat(shmfd, 0, 0);
	if (idxProd == (char*) -1) {
		perror("shmat");
		return SHMAT_ERR;
	}

    idxCons = idxProd + sizeof(char);
    shmbuf = idxCons + sizeof(char);
    

	//семафоры//
	int semfd = semget(IPC_PRIVATE, 3, IPC_CREAT | perms);
	errorCheck(semfd, -1, "semget", SEMGET_ERR);


	int s1 = semctl(semfd, BUFF_FULL, SETVAL, 0);
	errorCheck(s1, -1, "semctl", SEMCTL_ERR);

	int s2 = semctl(semfd, BUFF_EMPTY, SETVAL, BUFSIZE);
	errorCheck(s2, -1, "semctl", SEMCTL_ERR);

	int s3 = semctl(semfd, BIN_SEM, SETVAL, 1);
	errorCheck(s3, -1, "semctl", SEMCTL_ERR);

	
	signal(SIGINT, catchSignal);


	//процессы//
	//производители
	for (int i = 0; i < NumProd; i++) {
		pid_t pid = fork();
		errorCheck(pid, -1, "fork", FORK_ERR);

		if (pid == 0) {
			producer(semfd, i);
			exit(0);
		}
	}

	//потребители
	for (int i = 0; i < NumCons; i++) {
		pid_t pid = fork();
		errorCheck(pid, -1, "fork", FORK_ERR);

		if (pid == 0) {
			consumer(semfd, i);
			exit(0);
		}
	}


	//завершение//
	for (int i = 0; i < NumProd + NumCons; i++) {
		int status;
		wait(&status);
	}

	s1 = shmctl(shmfd, IPC_RMID, NULL);
	errorCheck(s1, -1, "shmctl", SHMCTL_ERR);

	s2 = semctl(semfd, BIN_SEM, IPC_RMID, 0);
	errorCheck(s2, -1, "semctl", SEMCTL_ERR);


	return 0;
}

void producer(const int semfd, const int id)
{
	while(!wasSignal) {

		int op1 = semop(semfd, prodP, 2);
		errorCheck(op1, -1, "Producer: semop1", SEMOP_ERR);

		shmbuf[*idxProd] = ALPHABET[*idxProd];
		printf("Producer %d:\t (pid %d)\t %c --> \n", id + 1, getpid(), ALPHABET[*idxProd]);
		(*idxProd)++;

		int op2 = semop(semfd, prodV, 2);
		errorCheck(op2, -1, "Producer: semop2", SEMOP_ERR);

		sleep(rand() % 3);
	}
}

void consumer(const int semfd, const int id)
{
	while(!isEndConsume) {

		int op1 = semop(semfd, consP, 2);
		errorCheck(op1, -1, "Consumer: semop1", SEMOP_ERR); 

		printf("Consumer %d:\t (pid %d)\t <-- %c\n", id + 1, getpid(), shmbuf[*idxCons]);
		(*idxCons)++;

		int op2 = semop(semfd, consV, 2);
		errorCheck(op2, -1, "Consumer: semop2", SEMOP_ERR); 

		sleep(rand() % 5);

		isEndConsume = (*idxProd == *idxCons);
	}
}


void errorCheck(int fd, int err, char* msg, int exCode)
{
	if (fd == err) {
		perror(msg);
		exit(exCode);
	}
}

void catchSignal(int signalNum)
{
	printf("");
	wasSignal = true;
}