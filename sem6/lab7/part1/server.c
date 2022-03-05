#include <signal.h>
#include "sockinc.h"

void close_socket();
void signal_handler(int signum);


static int sock;

void close_socket() 
{
    close(sock);
    unlink(SOCK_NAME);
}

void signal_handler(int signum) 
{
    printf("\nCatched signal CTRL+C\n");
    printf("Socket will close\n");
    close_socket();
    exit(0);
}


int main() 
{
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    struct sockaddr addr = {
        .sa_family = AF_UNIX,
        .sa_data = SOCK_NAME
    };

    if (bind(sock, &addr, sizeof(addr)) == -1) {
        perror("bind failed");
        return EXIT_FAILURE;
    }


    signal(SIGINT, signal_handler);

    printf("Wait msg from client...\n\n");

    char msg[MSG_LEN];
    while (1) {
        ssize_t bytes = recvfrom(sock, msg, sizeof(msg), 0, NULL, NULL);
        if (bytes == -1) {
            close_socket();
            perror("recv error");
            return EXIT_FAILURE;
        }

        msg[bytes] = '\n';
        msg[bytes + 1] = 0;
        printf("Msg from client: %s", msg);
    }
}