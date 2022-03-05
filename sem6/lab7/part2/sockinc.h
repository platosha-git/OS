#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

#include <arpa/inet.h>
#include <netdb.h>

#define HOST "localhost"
#define PORT 5120
#define MSG_LEN 256
#define MAX_CLIENTS 10
