// Simple UNIX domain stream socket echo server
#include "practice/us_xfr.h"
#include <sys/socket.h>
#include <sys/un.h>

#define BACKLOG 5
#define BUF_SIZE 100
#define SV_SOCK_PATH "/tmp/us_xfr"

int main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    int sfd, cfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Construct server socket address, bind socket to it, and make this a listening socket
    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT) {
        perror("remove");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sfd, BACKLOG) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Handle client connections iteratively
    for (;;) {
        // Accept a connection. The connection is returned on a new socket, 'cfd';
        // the listening socket ('sfd') remains open and can be used to accept further connections.
        cfd = accept(sfd, NULL, NULL);
        fprintf(stderr, "found client");
        if (cfd == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Transfer data from connected socket back to the client until EOF
        while ((numRead = read(cfd, buf, BUF_SIZE)) > 0) {
            if (write(cfd, buf, numRead) != numRead) {
                perror("partial/failed write");
                exit(EXIT_FAILURE);
            }
        }

        if (numRead == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        if (close(cfd) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
    }
}
