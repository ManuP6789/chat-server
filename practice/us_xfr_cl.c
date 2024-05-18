// Program from Michael Kerrisk from the Linux programming interface
// Simple UNIX domain stream socket server
// This is the client side program
#include "us_xfr.h"

#define BACKLOG 5

int main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    int sfd, cfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sfd == -1) {
        fprintf(stderr, "socket");
        exit(EXIT_FAILURE);
    }

    // Construct server socket address, bind socket to it, and make this a 
    // listening socket
    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT) {
        fprintf(stderr, "remove-%s", SV_SOCK_PATH);
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        fprintf(stderr, "connect");
        exit(EXIT_FAILURE);
    }

    // Copy stdin to socket
    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        if (write(sfd, buf, numRead) != numRead) {
            fprintf(stderr, "partial/failed write");
            exit(EXIT_FAILURE);
        }
    }

    if (numRead == -1) {
        fprintf(stderr, "read");
        exit(EXIT_FAILURE);

    }

    // CLoses our socket; server sees EOF
    return 0;
}