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


static int* shmbuf = NULL;


enum Semaphores
{
	ACTIVE_WR,
	WAITING_WR,
	
	ACTIVE_RD,
	WAITING_RD
};

struct sembuf startWrite[5] = {{WAITING_WR, 1, 0},
								{ACTIVE_RD, 0, 0},
								{ACTIVE_WR, 0, 0},
								{ACTIVE_WR, 1, 0},
								{WAITING_WR, -1, 0}};

struct sembuf stopWrite[1] = {{ACTIVE_WR, -1, 0}};

struct sembuf startRead[5] = {{WAITING_RD, 1, 0},
								{ACTIVE_WR, 0, 0},
								{WAITING_WR, 0, 0},
								{ACTIVE_RD, 1, 0},
								{WAITING_RD, -1, 0}};

struct sembuf stopRead[1] = {{ACTIVE_RD, -1, 0}};


const int perms = S_IRWXU | S_IRWXG | S_IRWXO;


const int NumReaders = 5;
const int NumWriters = 3;
const int LastRecord = 15;

static bool isEndRead = false;
static bool isEndWrite = false;


void writer(const int semfd, const int id);
void reader(const int semfd, const int id);
void errorCheck(int fd, int err, char* msg, int exCode);

int main()
{
	//память//
	int size = sizeof(int);
	int shmfd = shmget(IPC_PRIVATE, size, IPC_CREAT | perms);
	errorCheck(shmfd, -1, "shmget", SHMGET_ERR);

	shmbuf = (int*) shmat(shmfd, 0, 0);
	if (shmbuf == (int*) -1) {
		perror("shmat");
		return SHMAT_ERR;
	}

    (*shmbuf) = 0;


	//семафоры//
	int semfd = semget(IPC_PRIVATE, 4, IPC_CREAT | perms);
	errorCheck(semfd, -1, "semget", SEMGET_ERR);


	int s1 = semctl(semfd, ACTIVE_WR, SETVAL, 0);
	errorCheck(s1, -1, "semctl", SEMCTL_ERR);

	int s2 = semctl(semfd, WAITING_WR, SETVAL, 0);
	errorCheck(s2, -1, "semctl", SEMCTL_ERR);

	int s3 = semctl(semfd, ACTIVE_RD, SETVAL, 0);
	errorCheck(s3, -1, "semctl", SEMCTL_ERR);

	int s4 = semctl(semfd, WAITING_RD, SETVAL, 0);
	errorCheck(s4, -1, "semctl", SEMCTL_ERR);


	//процессы//
	//писатели
	for (int i = 0; i < NumWriters; i++) {
		pid_t pid = fork();
		errorCheck(pid, -1, "fork", FORK_ERR);

		if (pid == 0) {
			writer(semfd, i);
			exit(0);
		}
	}

	//читатели
	for (int i = 0; i < NumReaders; i++) {
		pid_t pid = fork();
		errorCheck(pid, -1, "fork", FORK_ERR);

		if (pid == 0) {
			reader(semfd, i);
			exit(0);
		}
	}


	//завершение//
	for (int i = 0; i < NumWriters + NumReaders; i++) {
		int status;
		wait(&status);
	}

	int s = shmdt(shmbuf);
	errorCheck(s, -1, "shmdt", SHMDT_ERR);

	return 0;
}

void writer(const int semfd, const int id)
{
	while(!isEndWrite) {

		int op1 = semop(semfd, startWrite, 5);
		errorCheck(op1, -1, "Writer: semop1", SEMOP_ERR);

		if ((*shmbuf) < LastRecord) {
			printf("Writer %d:\t (pid %d)\t %d --> \n", id + 1, getpid(), ++(*shmbuf));
		}
		isEndWrite = ((*shmbuf) >= LastRecord);

		int op2 = semop(semfd, stopWrite, 1);
		errorCheck(op2, -1, "Producer: semop3", SEMOP_ERR);

		sleep((rand() % 3) + 1);
	}
}

void reader(const int semfd, const int id)
{
	while(!isEndRead) {

		int op1 = semop(semfd, startRead, 5);
		errorCheck(op1, -1, "Reader: semop1", SEMOP_ERR); 

		printf("Reader %d:\t (pid %d)\t <-- %d\n", id + 1, getpid(), *shmbuf);
		isEndRead = ((*shmbuf) >= LastRecord);

		int op2 = semop(semfd, stopRead, 1);
		errorCheck(op2, -1, "Reader: semop2", SEMOP_ERR); 

		sleep((rand() % 6) + 1);
	}
}


void errorCheck(int fd, int err, char* msg, int exCode)
{
	if (fd == err) {
		perror(msg);
		exit(exCode);
	}
}