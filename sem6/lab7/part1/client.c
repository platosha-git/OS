#include "sockinc.h"

int main(int argc, char **argv) 
{
    if (argc != 2) {
        perror("wrong num of args");
        return EXIT_FAILURE;
    }

    char msg[MSG_LEN];
    strcpy(msg, argv[1]);


    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    //связывание безымянного сокета с адресом
    struct sockaddr addr = {
        .sa_family = AF_UNIX,
        .sa_data = SOCK_NAME
    };


    ssize_t err = sendto(sock, msg, strlen(msg), 0, &addr, sizeof(addr));
    if (err == -1) {
        perror("sendto failed");
        return EXIT_FAILURE;
    }

    close(sock);
    return 0;
}