#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "stat.h"

#define BUF_SIZE 0x1000
#define PATH_SIZE 30
#define PID 4559

int readCmdline(FILE *dest);
int readCwd(FILE *dest);
int readEnviron(FILE *dest);
int readExe(FILE *dest);
int readFd(FILE *dest);
int readMaps(FILE *dest);
int readRoot(FILE *dest);
int readStat(FILE *dest);


int main(int argc, char *argv[])
{
	FILE *myFile = fopen("myFile.txt", "w");

	readCmdline(myFile);
	readCwd(myFile);
	readEnviron(myFile);
	readExe(myFile);
	readFd(myFile);
	readMaps(myFile);
	readRoot(myFile);
	readStat(myFile);
	
	fclose(myFile);

	return 0;
}


//файл, указывает на директорию процесса
int readCmdline(FILE *dest)
{
	char path[PATH_SIZE];
	snprintf(path, 19, "/proc/%d/cmdline", PID);

	fprintf(dest, "\n-cmdline:\n");

	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(dest, "Can't open file %s\n", path);
		return -1;
	}

	char buf[BUF_SIZE];
	int len = fread(buf, 1, BUF_SIZE, f);
	buf[len] = 0;

	fprintf(dest, "\tComplete command line - %s\n", buf);
	fclose(f);

	return 0;
}


//символическая ссылка на директорию процесса
int readCwd(FILE *dest)
{
	char path[PATH_SIZE];
	snprintf(path, 15, "/proc/%d/cwd", PID);

	fprintf(dest, "\n-cwd:\n");

	char buf[BUF_SIZE];
	int len = readlink(path, buf, BUF_SIZE);
	if (!len) {
		fprintf(dest, "Can't read file %s\n", path);
		return -1;
	}
	buf[len] = 0;

	fprintf(dest, "\tPathname of current working directory - %s\n", buf);
	
	return 0;
}


//файл
int readEnviron(FILE *dest)
{
	char path[PATH_SIZE];
	snprintf(path, 19, "/proc/%d/environ", PID);
	
	fprintf(dest, "\n-environ:\n");

	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(dest, "Can't open file %s\n", path);
		return -1;
	}

	char buf[BUF_SIZE];
	int len = 0;
	while ((len = fread(buf, 1, BUF_SIZE, f)) > 0) {
		for (int i = 0; i < len; i++) {
			if (buf[i] == 0) {
				buf[i] = 10;
			}
		}
		buf[len] = 0; 
		fprintf(dest, "%s\n", buf);
	}

	fclose(f);

	return 0;
}


//символическая ссылка на файл процесса
int readExe(FILE *dest)
{
	char path[PATH_SIZE];
	snprintf(path, 19, "/proc/%d/exe", PID);

	fprintf(dest, "\n-exe:\n");

	char buf[BUF_SIZE];
	int len = readlink(path, buf, BUF_SIZE);
	if (!len) {
		fprintf(dest, "Can't read file %s\n", path);
		return -1;
	}
	buf[len] = 0;

	fprintf(dest, "\tPathname of the executed programm - %s\n", buf);

	return 0;
}


//файл-директория
int readFd(FILE *dest)
{
	char path[PATH_SIZE];
	snprintf(path, 14, "/proc/%d/fd", PID);

	fprintf(dest, "\n-fd:\n");


	DIR *dp = opendir(path); // open directory
	if (!dp) {
		fprintf(dest, "Can't open file%s\n", path);
		return -1;
	}
	
	struct dirent *dirp;
	char path_to_file[BUF_SIZE];
	char link_to_file[BUF_SIZE];
	int len;

	while((dirp = readdir(dp)) != NULL) {   
		if((strcmp(dirp->d_name, ".") !=0 ) && (strcmp(dirp->d_name, "..") != 0)) {
			len = sprintf(path_to_file, "%s/%s", path, dirp->d_name);
			path_to_file[len] = 0;

			len = readlink(path_to_file, link_to_file, BUF_SIZE);
			link_to_file[len] = 0;

			fprintf(dest, "\tFile: %s\tInode: %ld\tLink: %s\n", path_to_file, dirp->d_ino, link_to_file);
		}
	}

	closedir(dp);
	return 0;
}


//файл со списком выделенных участков памяти
int readMaps(FILE *dest)
{
	char path[PATH_SIZE];
	snprintf(path, 16, "/proc/%d/maps", PID);

	fprintf(dest, "\n-maps:\n");

	char buf[BUF_SIZE];
	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(dest, "Can't open file %s\n", path);
		return -1;
	}

	int len = 0;
	while ((len = fread(buf, 1, BUF_SIZE, f)) > 0) {
		buf[len] = 0; 
		fprintf(dest, "%s\n", buf);
	}

	fclose(f);

	return 0;
}


//soft link на корень ФС
int readRoot(FILE *dest)
{
	char path[PATH_SIZE];
	snprintf(path, 20, "/proc/%d/root", PID);

	fprintf(dest, "\n-root:\n");

	char buf[BUF_SIZE];
	int len = readlink(path, buf, BUF_SIZE);
	if (!len) {
		fprintf(dest, "Can't read file %s\n", path);
		return -1;
	}
	buf[len] = 0;
	
	fprintf(dest, "\tPer-process root of the filesystem: %s\n", buf);

	return 0;
}


int readStat(FILE *dest)
{
	char path[PATH_SIZE];
	snprintf(path, 16, "/proc/%d/stat", PID);

	fprintf(dest, "\n-stat:\n");

	FILE *f = fopen(path,"r");
	if (!f) {    
		fprintf(dest, "Can't open file %s\n", path);
		return -1;
	}

	char buf[BUF_SIZE];
	int len = fread(buf, 1, BUF_SIZE, f);
	buf[len] = '\0';

	int i = 0;
	char *pch = strtok(buf, " ");
	while(pch != NULL) {
		fprintf(dest, "\t%s:\t%s\n", statFields[i], pch);
		pch = strtok(NULL, " ");
		i++;
	}

	fclose(f);

	return 0;
}