#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>

#define MAX_EVENTS 64
#define MAX_SERVERS 3
#define PORT 9000
#define BUFFER_SIZE 4096

typedef struct {
    char ip[16];
    int port;
    int active_connections;
} Server;

Server servers[MAX_SERVERS] = {
    {"127.0.0.1", 8001, 0},
    {"127.0.0.1", 8002, 0},
    {"127.0.0.1", 8003, 0}
};

typedef struct {
    int fd;
    int peer_fd;
    int server_index;
} Conn;

int set_nonblocking(int fd) {
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

// greedy: least connections
int select_server() {
    int min = 0;
    for (int i = 1; i < MAX_SERVERS; i++) {
        if (servers[i].active_connections < servers[min].active_connections)
            min = i;
    }
    return min;
}

// BLOCKING connect (important!)
int connect_backend(int server_index) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(servers[server_index].port);
    inet_pton(AF_INET, servers[server_index].ip, &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    set_nonblocking(sock);
    return sock;
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(listen_fd);

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 128);

    int epfd = epoll_create1(0);

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    printf("Load balancer running on port %d...\n", PORT);

    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);

        for (int i = 0; i < n; i++) {

            // NEW CLIENT
            if (events[i].data.fd == listen_fd) {
                int client_fd = accept(listen_fd, NULL, NULL);
                if (client_fd < 0) continue;

                set_nonblocking(client_fd);

                int server_index = select_server();
                int backend_fd = connect_backend(server_index);

                if (backend_fd < 0) {
                    close(client_fd);
                    continue;
                }

                servers[server_index].active_connections++;

                // create two connection objects
                Conn *c1 = malloc(sizeof(Conn));
                Conn *c2 = malloc(sizeof(Conn));

                c1->fd = client_fd;
                c1->peer_fd = backend_fd;
                c1->server_index = server_index;

                c2->fd = backend_fd;
                c2->peer_fd = client_fd;
                c2->server_index = server_index;

                struct epoll_event ev1, ev2;

                ev1.events = EPOLLIN;
                ev1.data.ptr = c1;

                ev2.events = EPOLLIN;
                ev2.data.ptr = c2;

                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev1);
                epoll_ctl(epfd, EPOLL_CTL_ADD, backend_fd, &ev2);

                printf("Client → Server %d\n", server_index);
            }

            // DATA FORWARDING
            else {
                Conn *conn = (Conn *)events[i].data.ptr;

                char buffer[BUFFER_SIZE];
                int bytes = read(conn->fd, buffer, BUFFER_SIZE);

                if (bytes <= 0) {
                    close(conn->fd);
                    close(conn->peer_fd);

                    servers[conn->server_index].active_connections--;
                    free(conn);
                    continue;
                }

                int written = write(conn->peer_fd, buffer, bytes);
                if (written <= 0) {
                    close(conn->fd);
                    close(conn->peer_fd);

                    servers[conn->server_index].active_connections--;
                    free(conn);
                }
            }
        }
    }
}
