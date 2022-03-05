#include "sockinc.h"

int main(void)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    struct hostent *server = gethostbyname(HOST); 
    if (server == NULL) {
        perror("gethostbyname failed");
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr = *((struct in_addr*) server->h_addr_list[0]),
        .sin_port = htons(PORT)
    };
    

    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("connect failed");
        return EXIT_FAILURE;
    }

        
    char msg[MSG_LEN];
    for (size_t i = 0; i < 5; i++) {
        sprintf(msg, "num %ld", i+1);
        
        ssize_t err = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *) &addr, sizeof(addr));
        if (err == -1) {
            close(sock);
            perror("sendto failed");
            return EXIT_FAILURE;
        }
        
        printf("Sent num %ld\n", i + 1);
        sleep(rand() % 2 + 1);
    }

    return 0;
}
