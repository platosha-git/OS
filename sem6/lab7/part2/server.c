#include <signal.h>
#include "sockinc.h"

static int cur_clients = 0;
static int sock = 0;

void signal_handler(int sig) {
    printf("\nCatched signal CTRL+C\n");
    printf("Server will stop\n");

    close(sock);
    exit(0);
}

int new_client_handler(int* const clients)
{
    struct sockaddr_in addr;
    int addr_size = sizeof(addr);

    int newsock = accept(sock, (struct sockaddr*) &addr, (socklen_t*) &addr_size);
    if (newsock < 0) {
        perror("accept failed");
        return EXIT_FAILURE;
    }

    cur_clients++;
    printf("\nClient %d (fd = %d)\n\n", cur_clients, newsock);

    if (cur_clients < MAX_CLIENTS) {
        clients[cur_clients - 1] = newsock;
    }
    else {
        perror("too many clients");
        return EXIT_FAILURE;   
    }

    return 0;
}

void process_clients(fd_set *set, int* const clients)
{
    char msg[MSG_LEN] = { 0 };
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int fd = clients[i];
        if ((fd > 0) && FD_ISSET(fd, set)) {
            ssize_t read_len = read(fd, msg, MSG_LEN);
            if (read_len == -1) {
                perror("read failed");
            }
            else if (read_len == 0) {
                printf("\nClient %d disconnected\n\n", i+1);
                close(fd);
                clients[i] = 0;
            }
            else {
                msg[read_len] = '\0';
                printf("Msg from client %d: %s\n", i+1, msg);
            }
        }
    }   
}


int main(void)
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY    
    };

    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind failed");
        return EXIT_FAILURE;
    }

    signal(SIGINT, signal_handler);

    if (listen(sock, 3) < 0) {
        perror("listen failed");
        return EXIT_FAILURE;
    }
        

    printf("Listen port %d.\n", PORT);

    int clients[MAX_CLIENTS] = { 0 };
    while (1) {
        fd_set set; 
        FD_ZERO(&set);
        FD_SET(sock, &set);
        int max_fd = sock;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int cur_cl = clients[i]; 
            if (cur_cl > 0) {
                FD_SET(cur_cl, &set);
            }

            max_fd = (cur_cl > max_fd) ? (cur_cl) : (max_fd);
        }
        

        struct timeval timeout = {1, 500000};
        int activ = select(max_fd + 1, &set, NULL, NULL, &timeout);
        if (activ < 0) {
            perror("select failed");
            return EXIT_FAILURE;
        }
        
        if (FD_ISSET(sock, &set)) {
            int err = new_client_handler(clients);
            if (err != 0) {
                return err;
            }
        }

        process_clients(&set, clients);
    }

    return 0;
}
