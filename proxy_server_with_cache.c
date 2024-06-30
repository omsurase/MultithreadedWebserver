#include "proxy_parse.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef struct cache_element cache_element;
#define MAX_CLIENTS 10
#define MAX_BYTES 4096
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

int connectRemoteServer(char *host_addr, int port_num)
{
    int remoteSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (remoteSocket < 0)
    {
        printf("Erro in creating your socket");
        return -1;
    }

    struct hostent *host = gethostbyname(host_addr);
    if (host == NULL)
    {
        fprintf(stderr, "No such host exist\n");
        return -1;
    }
    struct sockaddr_in server_addr;
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_num);
    bcopy((char *)host->h_addr_list[0], (char *)&server_addr.sin_addr.s_addr, host->h_length);

    if (connect(remoteSocket, (struct sockaddr *)&server_addr, (socklen_t)sizeof(server_addr)) < 0)
    {
        fprintf(stderr, "Error in connecting !\n");
        return -1;
    }
    remote remoteSocket;
}

int handle_request(int clientSocketId, ParsedRequest *request, char *tempReq)
{
    char *buff = (char *)malloc(sizeof(char) * MAX_BYTES);
    strcpy(buff, "GET");
    strcat(buff, request->path);
    strcat(buff, " ");
    strcat(buff, request->version);
    strcat(buff, "\r\n");
    size_t len = strlen(buff);

    if (ParsedHeader_set(request, "Connection", "close") < 0)
    {
        printf("set header key is not working");
    }

    if (ParsedHeader_get(request, "Host") == NULL)
    {
        if (ParsedHeader_set(request, "Host", request->host) < 0)
        {
            printf("Set Host header key is not working");
        }
    }

    if (ParsedRequest_unparse_headers(request, buff + len, (size_t)MAX_BYTES - len) < 0)
    {
        printf("unparse failed");
    }
    int server_port = 80;
    if (request->port != NULL)
    {
        server_port = atoi(request->port);
    }

    int remoteSocketId = connectRemoteServer(request->host, server_port);
}

void *thread_fn(void *socketNew)
{
    sem_wait(&semaphore); // decreases semaphore by 1 and checks if value is negetive. if negative thread keeps waiting here
    int p;
    sem_getvalue(&semaphore, p);
    printf("Semaphore value is %d", p);
    int *t = (int *)socketNew;
    int socket = *t;
    int byte_send_by_client, len;
    char *buffer = (char *)calloc(MAX_BYTES, sizeof(char));
    bzero(buffer, MAX_BYTES);
    byte_send_by_client = recv(socket, buffer, MAX_BYTES, 0);

    while (byte_send_by_client > 0)
    {
        len = strlen(buffer);
        if (strstr(buffer, "\r\n\r\n") == NULL)
        {
            byte_send_by_client = recv(socket, buffer + len, MAX_BYTES - len, 0);
        }
        else
        {
            break;
        }
    }

    char *tempReq = (char *)malloc(strlen(buffer) * sizeof(char) + 1);
    for (int i = 0; i < strlen(buffer); i++)
    {
        tempReq[i] = buffer[i];
    }

    struct cache_element *temp = find(tempReq);

    if (temp != NULL)
    {
        int size = temp->len / sizeof(char);
        int pos = 0;
        char response[MAX_BYTES];
        while (pos < size)
        {
            for (int i = 0; i < MAX_BYTES; i++)
            {
                response[i] = temp->data[i];
                pos++;
            }
            send(socket, response, MAX_BYTES, 0);
        }
        printf("Data retrieved from cache\n");
        print("\n\n%s\n\n", response);
    }
    else if (byte_send_by_client > 0)
    {
        len = strlen(buffer);
        ParsedRequest *request = ParsedRequest_create();
        if (ParsedRequest_parse(request, buffer, len) < 0)
        {
            printf("Parsing failed.");
        }
        else
        {
            bzero(buffer, MAX_BYTES);
            if (!strcmp(request->method, "GET"))
            {
                if (request->host && request->path && checkHTTPversion(request->version) == 1)
                {
                    byte_send_by_client = handle_request(socket, request, tempReq);
                    if (byte_send_by_client == -1)
                    {
                        sendErrorMessage(socket, 500);
                    }
                }
                else
                {
                    sendErrorMessage(socket, 500);
                }
            }
            else
            {
                printF("Methods other than GET are not supported");
            }
        }
        ParsedRequest_destroy(request);
    }
    else if (byte_send_by_client == 0)
    {
        printf("Client is disconnected");
    }
    shutdown(socket, SHUT_RDWR);
    close(socket);
    free(buffer);
    sem_post(&semaphore);
    sem_getvalue(&semaphore, p);
    printf("Semaphore post value is %d\n");
    free(tempReq);
    return NULL;
}

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

    int i = 0;
    int Connected_socketId[MAX_CLIENTS];

    while (1)
    {
        bzero((char *)&client_addr, sizeof(client_addr));
        client_len = sizeof(client_addr);
        client_socketId = accept(proxy_socketId, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
        if (client_socketId < 0)
        {
            perro("Not able to connect");
            exit(1);
        }
        else
        {
            Connected_socketId[i] = client_socketId;
        }

        struct sockaddr_in *client_pt = (struct sockaddr_in *)&client_addr;
        struct in_addr ip_addr = client_pt->sin_addr;
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip_addr, str, INET6_ADDRSTRLEN);
        printf("Client is connnected with port number %d and ip address is %s\n", ntohs(client_addr.sin_port), str);

        pthread_create(&tid[i], NULL, thread_fn, (void *)&Connected_socketId);
        i++;
    }
    close(proxy_socketId);
    return 0;
}