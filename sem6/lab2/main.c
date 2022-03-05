#include "apue.h"
#include <dirent.h>
#include <limits.h>

enum FileTypes
{
	FTW_F,		// файл, не являющийся каталогом
	FTW_D,		// каталог
	FTW_DNR,	// каталог, который не доступен для чтения
	FTW_NS		// файл, информацию о котором невозможно получить с помощью stat
};


typedef int Myfunc(const char *, int, int);

static Myfunc myfunc;
static int myftw(char *, Myfunc *);
static int dopath(Myfunc *func, int depth);


static char fullpath[PATH_MAX + 1];


int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Использование: ./a.out <начальный_каталог>!\n");
		exit(1);
	}

	int ret = myftw(argv[1], myfunc);

	return ret;
}


static int myftw(char *pathname, Myfunc *myfunc)
{
	if (chdir(pathname) != 0) {
		printf("Ошибка вызова функции chdir %s!\n", pathname);
		exit(1);
	}
	
	strcpy(fullpath, pathname);
	int ret = dopath(myfunc, 0);

	return ret;
}


static int dopath(Myfunc *func, int depth)
{
	struct stat statbuf;

	// проверка вызова lstat
	if (lstat(fullpath, &statbuf) < 0) {
		return 1;
	}
	
	// проверка, что католог
	if (S_ISDIR(statbuf.st_mode) == 0) {
		return func(fullpath, FTW_F, depth);
	}
	

	int ret = 0;
	if ((ret = func(fullpath, FTW_D, depth)) != 0) {
		return ret;
	}


	DIR *dp = NULL;
	// проверка доступа к катологу 
	if ((dp = opendir(fullpath)) == NULL) {
		return 1;
	} 

	if (chdir(fullpath) < 0) {
		printf("Ошибка вызова функции chdir %s!\n", fullpath);
		return 1;
	}

	
	struct dirent *dirp = NULL;
	while ((dirp = readdir(dp)) != NULL) {
		if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0) {
			strcpy(fullpath, dirp->d_name);
			dopath(func, depth + 1);
		}
	}

	if (chdir("..") != 0) {
		printf("Ошибка вызова функции chdir ..!\n");
		return 1;
	}

	if (closedir(dp) < 0) {
		printf("Невозможно закрыть каталог %s!\n", fullpath);
		return 1;
	}

	return ret;
}

static int myfunc(const char *pathname, int type, int depth)
{
	switch (type) {
	case FTW_F:
		for (int i = 0; i < depth; i++) {
			printf("\t");
		}
		
		printf("-| %s\n", pathname);
		break;

	case FTW_D:
		for (int i = 0; i < depth; i++) {
			printf("\t");
		}

		printf("|-->%s\n", pathname);
		break;

	case FTW_DNR:
        printf("Закрыт доступ к каталогу %s\n!", pathname); 
        return 1;
    
    case FTW_NS:
        printf("Ошибка функции stat!\n");
        return 1;
        
    default: 
        printf("Неизвестый тип файла %s\n!", pathname);
        return 1;
	}

	return 0;
}

