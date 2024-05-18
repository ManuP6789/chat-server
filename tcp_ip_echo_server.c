// Simple UNIX domain stream socket server
#include "practice/us_xfr.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define PORT 12345
#define BACKLOG 5

int clients[BACKLOG]; // Array to store client socket descriptors
int num_clients = 0;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex to protect the clients array

typedef struct message {
    int sender_fd;
    char text[BUF_SIZE];
} message_t;

message_t message_queue[BUF_SIZE]; // Simple circular buffer for the message queue
int queue_head = 0;
int queue_tail = 0;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex to protect the message queue
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER; 

void *handle_client(void *arg);
void *broadcast_thread(void *arg);
void enqueue_message(int sender_fd, const char *msg);

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int sfd, *cfd;
    pthread_t tid, broadcast_tid;

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        fprintf(stderr, "socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sfd, BACKLOG) == -1) {
        fprintf(stderr, "listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    if (pthread_create(&broadcast_tid, NULL, broadcast_thread, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    // Handle client connections iteratively
    for (;;) {
        cfd = malloc(sizeof(int));
        if (!cfd) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        *cfd = accept(sfd, NULL, NULL);
        if(*cfd == -1) {
            perror("accept");
            free(cfd);
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (num_clients < BACKLOG) {
            clients[num_clients++] = *cfd;
            pthread_mutex_unlock(&clients_mutex);

             if (pthread_create(&tid, NULL, handle_client, cfd) != 0) {
                fprintf(stderr, "pthread_create");
                close(*cfd);
                free(cfd);
            } else {
                fprintf(stderr, "user connected\n");
                pthread_detach(tid);
            }


        } else {
            pthread_mutex_unlock(&clients_mutex);
            fprintf(stderr, "Maximum number of clients reached\n");
            close(*cfd);
            free(cfd);
        }
    }
}



void *handle_client(void *arg) {
    int cfd = *((int *)arg);
    free(arg);
    char buf[BUF_SIZE];
    ssize_t numRead;

    // Transfer data from connected socket back to the client until EOF

    while ((numRead = read(cfd, buf, BUF_SIZE)) > 0) {
        if (write(cfd, buf, numRead) != numRead) {
            fprintf(stderr, "partial/failed write");
            close(cfd);
            return NULL;
        }
        buf[numRead] = '\0';
        enqueue_message(cfd, buf);  
    }

    if (numRead == -1) {
        perror("read");
    }

    if (close(cfd) == -1) {
        perror("close");
    }

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i] == cfd) {
            clients[i] = clients[num_clients - 1];
            num_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    return NULL;
}

void *broadcast_thread(void *arg) {
    (void)arg; // Suppress unused parameter warning
    while (1) {
        pthread_mutex_lock(&queue_mutex);
        while (queue_head == queue_tail) {
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }

        message_t msg = message_queue[queue_tail];
        queue_tail = (queue_tail + 1) % BUF_SIZE;
        pthread_mutex_unlock(&queue_mutex);

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < num_clients; i++) {
            if (clients[i] != msg.sender_fd) {
                if (send(clients[i], msg.text, strlen(msg.text), 0) == -1) {
                    perror("broadcast error");
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

void enqueue_message(int sender_fd, const char *msg) {
    pthread_mutex_lock(&queue_mutex);
    message_queue[queue_head].sender_fd = sender_fd;
    strncpy(message_queue[queue_head].text, msg, BUF_SIZE);
    queue_head = (queue_head + 1) % BUF_SIZE;
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);
}