#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

const int sizeMsg = 48;

void waitStatus()
{
	int status;
	pid_t childpid = wait(&status);

	if (WIFEXITED(status)) {
		printf("Child %d exited with code: %d\n", childpid, WEXITSTATUS(status));
	}
	else if (WIFSIGNALED(status)) {
		printf("Child %d ended on signal: %d\n", childpid, WTERMSIG(status));
	}
	else if (WIFSTOPPED(status)) {
		printf("Child %d stopped on signal: %d\n", childpid, WSTOPSIG(status));
	}
}


int main()
{
	int fd[2];
	if (pipe(fd) == -1) {
		perror("Can`t pipe");
		exit(1);
	}

	//Потомок 1
	pid_t childpid1 = fork();

	if (childpid1 == -1) {
		perror("Can`t fork");
		exit(1);
	} 
	else if (childpid1 == 0) {
		char msg1[] = "Message from first child!\n";

		close(fd[0]);
		write(fd[1], msg1, strlen(msg1));
		
		exit(0);
	}


	//Потомок 2
	pid_t childpid2 = fork();

	if (childpid2 == -1) {
		perror("Can`t fork");
		exit(1);
	} 
	else if (childpid2 == 0) {
		char msg2[] = "Message from child 2!\n";

		close(fd[0]);
		write(fd[1], msg2, strlen(msg2));

		exit(0);
	}

	
	//Предок
	printf("Parent: pid = %d, pgrp = %d, childpid1 = %d, childpid2 = %d\n\n", 
					getpid(), getpgrp(), childpid1, childpid2);

	waitStatus();
	waitStatus();

	printf("\nMsgs from pipe:\n");
	char msg[sizeMsg];

	close(fd[1]);
	read(fd[0], msg, sizeMsg);

	printf("%s\n", msg);		

	return 0;
}
