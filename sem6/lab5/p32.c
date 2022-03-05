#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pthread.h>

#define wrAlphabet "wrAlphabet.txt"

void *thread_write(int *fd)
{
	struct stat sb;

	FILE *fp = fopen(wrAlphabet, "w");
	stat(wrAlphabet, &sb);
	printf("Open\n\tFp%d:\tinode - %d\tsize - %d\n", *fd, (int) sb.st_ino, (int) sb.st_size);

	for (char c = 'a'; c <= 'z'; c++) {
		if (c % 2 == 0 && *fd % 2 == 1) {
			fprintf(fp, "%c", c);
		}
		
		if (c % 2 == 1 && *fd % 2 == 0) {
			fprintf(fp, "%c", c);	
		}
	}

	fclose(fp);
	stat(wrAlphabet, &sb);
	printf("Close\n\tFp%d:\tinode - %d\tsize - %d\n", *fd, (int) sb.st_ino, (int) sb.st_size);
}

int main()
{
	pthread_t thread1, thread2;
	int fd1 = 1, fd2 = 2;

	int stat1 = pthread_create(&thread1, NULL, thread_write, &fd1);
	if (stat1 != 0) {
		printf("Error. Can`t create thread 1!\n");
		exit(1);
	}

	int stat2 = pthread_create(&thread2, NULL, thread_write, &fd2);
	if (stat2 != 0) {
		printf("Error. Can`t create thread 1!\n");
		exit(1);
	}

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	return 0;
}