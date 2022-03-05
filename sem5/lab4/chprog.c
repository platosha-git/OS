#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	printf("\nThis programm run by child %d!\n", atoi(argv[1]));
	return 0;
}