#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char *argv[])
{
	pid_t pid = getpid();
	printf("pid = %d\n", pid);

	int wait = 0;

	scanf("%d", &wait);

	double res = 0;
	for (int i = 0; i < 100000000; i++) {
		res = sin(pow(2, i)) * sin(pow(2, i + 1));
	};

	return 0;
}