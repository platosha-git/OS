#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/file.h>

int main()
{
	syslog(LOG_DEBUG, "Hello world!\n");

	return 0;
}