#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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
	//Потомок 1
	pid_t childpid1 = fork();

	if (childpid1 == -1) {
		perror("Can`t fork");
		exit(1);
	}
	else if (childpid1 == 0) {
		printf("Child1: pid = %d, ppid = %d, pgrp = %d\n",
						getpid(), getppid(), getpgrp());
		exit(0);
	}


	//Потомок 2
	pid_t childpid2 = fork();

	if (childpid2 == -1) {
		perror("Can`t fork");
		exit(1);
	}
	else if (childpid2 == 0) {
		printf("Child2: pid = %d, ppid = %d, pgrp = %d\n",
						getpid(), getppid(), getpgrp());
		exit(0);
	}


	//Предок
	printf("Parent: pid = %d, pgrp = %d, childpid1 = %d, childpid2 = %d\n\n",
					getpid(), getpgrp(), childpid1, childpid2);

	waitStatus();
	waitStatus();

	return 0;
}
