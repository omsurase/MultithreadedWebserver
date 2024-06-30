#include "proxy_parse.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct cache_element cache_element;
#define MAX_CLIENTS 10

// element inside LRU cache.
struct cache_element
{
    char *data;            // data present in response of a respective requests.
    int len;               // len of data (no of bytes).
    char *url;             // url from where response was requested (used for searching).
    time_t lru_time_track; // time when element was inserted in cache.
    cache_element *next;   // pointer to next element in cache.
};

cache_element *find(char *url);                        // find a element in cache.
int add_cache_element(char *data, int len, char *url); // add an element to cache.
void remove_cache_element();                           // remove an element from cache.

int port_number = 8080;
int proxy_socketId;
pthread_t tid[MAX_CLIENTS]; // at any moment at max 10 threads will be alive to handle 10 clients concurrently.
sem_t semaphore;
pthread_mutex_t lock;

cache_element *head; // global head of LRU cache
int cache_size;

int main(int argc, char *argv[])
{
    int client_socketId, client_len;
    struct sockaddr_in server_addr, client_addr;
    sem_int(&semaphore, MAX_CLIENTS);
    pthread_mutex_init(&lock, NULL);

    if (argv == 2)
    {
        // ./proxy  8090;
        port_number = atoi(argv[1]);
    }
    else
    {
        printf("too few arguements\n");
        exit(1);
    }

    printf("Start proxy server at port: %d\n", port_number);

    proxy_socketId = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_socketId < 0)
    {
        perror("Failed to create a socket\n");
        exit(1);
    }
    int reuse = 1; // same socket will be used to listen to all the clients.
    if (setsockopt(proxy_socketId, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0)
    {
        perror("setSockOpt failed");
    }

    bzero((char *)&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number); // network byte order and machine byte order translation.
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Binding the socket
    if (bind(proxy_socketId, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Port is not available\n");
        exit(1);
    }

    printf("Binding on port %d\n", port_number);

    // Proxy socket listening to the requests
    int listen_status = listen(proxy_socketId, MAX_CLIENTS);

    if (listen_status < 0)
    {
        perror("Error while Listening !\n");
        exit(1);
    }

    return 0;
}