#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    auto err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void do_something(int connfd) {
    char rbuf [64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        msg("read() error");
        return;
    }

    printf ("client says: %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf));
}

int main() {
    auto fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) die("socket()");

    // necessary for most server applications
    int val = 1;
    setsockopt(fd,SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0); // 0.0.0.0
    auto rv = bind(fd,(const sockaddr *)&addr, sizeof(addr));

    if (rv)
        die("bind()");

    // listen
    rv = listen(fd,SOMAXCONN);
    if (rv)
        die("listen()");
    
    while (true) {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        auto connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
        if (connfd < 0)
            // error
            continue;

        do_something(connfd);
        close(connfd);
    }
}