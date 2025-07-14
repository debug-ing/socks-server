#include "netinet/in.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>


void *client_worker(void *args) {
    int client_fd = (intptr_t) args;
    return NULL;
}


_Noreturn void server_loop(int server_fd) {
    while(1){
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	if (client_fd < 0) {
            perror("accept");
            continue;
        }
        int optval = 1;
        if (setsockopt(client_fd, SOL_TCP, TCP_NODELAY, &optval, sizeof(optval)) < 0) {
            perror("setsockopt");
	    close(client_fd);
            continue;
        }

	printf("Accepted connection from %s:%d with FD %d\n",
        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);
	pthread_t client_tid;
	if (pthread_create(&client_tid, NULL, &client_worker, (void *) (intptr_t) client_fd) == 0) {
            pthread_detach(client_tid);
        } else {
            perror("pthread_create");
            close(client_fd);
        }
    }
}
int main() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
	perror("Socket creation failed");
	return 1;
    }
    int optval = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt()");
        close(s);
        return 1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(1080); // Default SOCKS5 sin_port
    
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
	perror("Bind failed");
	close(s);
	return 1;
    }

    if (listen(s, 5) < 0) {
	perror("Listen failed");
	close(s);
	return 0;
    }

    printf("SOCKS5 server listening on port 1080\n");
    server_loop(s);
    return 0;
}

