#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	//Потомок 1
	pid_t childpid1 = fork();

	if (childpid1 == -1) {
		perror("Can`t fork");
		exit(1);
	}
	else if (childpid1 == 0) {
		printf("Before sleep. Child1: pid = %d, ppid = %d, pgrp = %d\n",
					  				  getpid(), getppid(), getpgrp());
		sleep(3);
		printf("After sleep. Child1: pid = %d, ppid = %d, pgrp = %d\n",
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
		printf("Before sleep. Child2: pid = %d, ppid = %d, pgrp = %d\n",
									  getpid(), getppid(), getpgrp());
		sleep(3);
		printf("After sleep. Child2: pid = %d, ppid = %d, pgrp = %d\n",
									 getpid(), getppid(), getpgrp());
		exit(0);
	}


	//Предок
	printf("Parent: pid = %d, pgrp = %d, childpid1 = %d, childpid2 = %d\n\n",
					getpid(), getpgrp(), childpid1, childpid2);

	return 0;
}
