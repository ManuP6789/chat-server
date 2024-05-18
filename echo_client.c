// Simple UNIX domain stream socket client
#include "practice/us_xfr.h"
#include <sys/socket.h>
#include <sys/un.h>

#define BUF_SIZE 100
#define SV_SOCK_PATH "/tmp/us_xfr"

int main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    int sfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // Copy stdin to socket and read back the response
    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        if (write(sfd, buf, numRead) != numRead) {
            perror("partial/failed write");
            exit(EXIT_FAILURE);
        }

        // Read the echoed response from the server
        numRead = read(sfd, buf, BUF_SIZE);
        if (numRead > 0) {
            if (write(STDOUT_FILENO, buf, numRead) != numRead) {
                perror("partial/failed write");
                exit(EXIT_FAILURE);
            }
        } else if (numRead == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
    }

    if (numRead == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // Close our socket; server sees EOF
    close(sfd);
    return 0;
}
