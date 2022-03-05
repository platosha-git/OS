#include <stdio.h>
#include <sys/stat.h>

#define wrAlphabet "wrAlphabet.txt"

int main()
{
	struct stat sb;

	printf("Open\n");

	FILE *fp1 = fopen(wrAlphabet, "w");
	stat(wrAlphabet, &sb);
	printf("\tFp1:\tinode - %d\tsize - %d\n", (int) sb.st_ino, (int) sb.st_size);

	FILE *fp2 = fopen(wrAlphabet, "w");
	stat(wrAlphabet, &sb);
	printf("\tFp2:\tinode - %d\tsize - %d\n", (int) sb.st_ino, (int) sb.st_size);


	for (char c = 'a'; c <= 'z'; c++) {
		if (c % 2 == 0) {
			fprintf(fp1, "%c", c);
		}
		else {
			fprintf(fp2, "%c", c);	
		}
	}

	printf("Close\n");
	
	fclose(fp1);
	stat(wrAlphabet, &sb);
	printf("\tFp1:\tinode - %d\tsize - %d\n", (int) sb.st_ino, (int) sb.st_size);

	fclose(fp2);
	stat(wrAlphabet, &sb);
	printf("\tFp2:\tinode - %d\tsize - %d\n", (int) sb.st_ino, (int) sb.st_size);

	return 0;
}