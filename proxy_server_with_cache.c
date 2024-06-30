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
#define MAX_SIZE 200 * (1 << 20) // size of the cache
#define MAX_ELEMENT_SIZE 10 * (1 << 20)
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

int sendErrorMessage(int socket, int status_code)
{
    char str[1024];
    char currentTime[50];
    time_t now = time(0);

    struct tm data = *gmtime(&now);
    strftime(currentTime, sizeof(currentTime), "%a, %d %b %Y %H:%M:%S %Z", &data);

    switch (status_code)
    {
    case 400:
        snprintf(str, sizeof(str), "HTTP/1.1 400 Bad Request\r\nContent-Length: 95\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>400 Bad Request</TITLE></HEAD>\n<BODY><H1>400 Bad Rqeuest</H1>\n</BODY></HTML>", currentTime);
        printf("400 Bad Request\n");
        send(socket, str, strlen(str), 0);
        break;

    case 403:
        snprintf(str, sizeof(str), "HTTP/1.1 403 Forbidden\r\nContent-Length: 112\r\nContent-Type: text/html\r\nConnection: keep-alive\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>403 Forbidden</TITLE></HEAD>\n<BODY><H1>403 Forbidden</H1><br>Permission Denied\n</BODY></HTML>", currentTime);
        printf("403 Forbidden\n");
        send(socket, str, strlen(str), 0);
        break;

    case 404:
        snprintf(str, sizeof(str), "HTTP/1.1 404 Not Found\r\nContent-Length: 91\r\nContent-Type: text/html\r\nConnection: keep-alive\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\n<BODY><H1>404 Not Found</H1>\n</BODY></HTML>", currentTime);
        printf("404 Not Found\n");
        send(socket, str, strlen(str), 0);
        break;

    case 500:
        snprintf(str, sizeof(str), "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 115\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>500 Internal Server Error</TITLE></HEAD>\n<BODY><H1>500 Internal Server Error</H1>\n</BODY></HTML>", currentTime);
        // printf("500 Internal Server Error\n");
        send(socket, str, strlen(str), 0);
        break;

    case 501:
        snprintf(str, sizeof(str), "HTTP/1.1 501 Not Implemented\r\nContent-Length: 103\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>404 Not Implemented</TITLE></HEAD>\n<BODY><H1>501 Not Implemented</H1>\n</BODY></HTML>", currentTime);
        printf("501 Not Implemented\n");
        send(socket, str, strlen(str), 0);
        break;

    case 505:
        snprintf(str, sizeof(str), "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Length: 125\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>505 HTTP Version Not Supported</TITLE></HEAD>\n<BODY><H1>505 HTTP Version Not Supported</H1>\n</BODY></HTML>", currentTime);
        printf("505 HTTP Version Not Supported\n");
        send(socket, str, strlen(str), 0);
        break;

    default:
        return -1;
    }
    return 1;
}

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
    return remoteSocket;
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

    if (remoteSocketId < 0)
    {
        return -1;
    }

    int bytes_send = send(remoteSocketId, buff, strlen(buff), 0);

    bzero(buff, MAX_BYTES);

    bytes_send = recv(remoteSocketId, buff, MAX_BYTES - 1, 0);
    char *temp_buffer = (char *)malloc(sizeof(char) * MAX_BYTES); // temp buffer
    int temp_buffer_size = MAX_BYTES;
    int temp_buffer_index = 0;

    while (bytes_send > 0)
    {
        // sending response from remote server to client socket
        bytes_send = send(clientSocketId, buff, bytes_send, 0);

        for (int i = 0; i < bytes_send / sizeof(char); i++)
        {
            // copying the response to tempbuff to be put into cache.
            temp_buffer[temp_buffer_index] = buff[i];
            // printf("%c",buf[i]); // Response Printing
            temp_buffer_index++;
        }
        // if memory of temp buff not enough reallocating it more memory
        temp_buffer_size += MAX_BYTES;
        temp_buffer = (char *)realloc(temp_buffer, temp_buffer_size);

        if (bytes_send < 0)
        {
            perror("Error in sending data to client socket.\n");
            break;
        }
        bzero(buff, MAX_BYTES);

        bytes_send = recv(remoteSocketId, buff, MAX_BYTES - 1, 0);
    }
    temp_buffer[temp_buffer_index] = '\0';
    free(buff);
    // adding element to lru cache.
    add_cache_element(temp_buffer, strlen(temp_buffer), tempReq);
    printf("Done\n");
    free(temp_buffer);

    close(remoteSocketId);
    return 0;
}

int checkHTTPversion(char *msg)
{
    int version = -1;

    if (strncmp(msg, "HTTP/1.1", 8) == 0)
    {
        version = 1;
    }
    else if (strncmp(msg, "HTTP/1.0", 8) == 0)
    {
        version = 1; // Handling this similar to version 1.1
    }
    else
        version = -1;

    return version;
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

cache_element *find(char *url)
{

    // Checks for url in the cache if found returns pointer to the respective cache element or else returns NULL
    cache_element *site = NULL;
    // sem_wait(&cache_lock);
    int temp_lock_val = pthread_mutex_lock(&lock);
    printf("Remove Cache Lock Acquired %d\n", temp_lock_val);
    if (head != NULL)
    {
        site = head;
        while (site != NULL)
        {
            if (!strcmp(site->url, url))
            {
                printf("LRU Time Track Before : %ld", site->lru_time_track);
                printf("\nurl found\n");
                // Updating the time_track
                site->lru_time_track = time(NULL);
                printf("LRU Time Track After : %ld", site->lru_time_track);
                break;
            }
            site = site->next;
        }
    }
    else
    {
        printf("\nurl not found\n");
    }
    // sem_post(&cache_lock);
    temp_lock_val = pthread_mutex_unlock(&lock);
    printf("Remove Cache Lock Unlocked %d\n", temp_lock_val);
    return site;
}

int add_cache_element(char *data, int size, char *url)
{
    // Adds element to the cache
    // sem_wait(&cache_lock);
    int temp_lock_val = pthread_mutex_lock(&lock);
    printf("Add Cache Lock Acquired %d\n", temp_lock_val);
    int element_size = size + 1 + strlen(url) + sizeof(cache_element); // Size of the new element which will be added to the cache
    if (element_size > MAX_ELEMENT_SIZE)
    {
        // sem_post(&cache_lock);
        //  If element size is greater than MAX_ELEMENT_SIZE we don't add the element to the cache
        temp_lock_val = pthread_mutex_unlock(&lock);
        printf("Add Cache Lock Unlocked %d\n", temp_lock_val);
        // free(data);
        // printf("--\n");
        // free(url);
        return 0;
    }
    else
    {
        while (cache_size + element_size > MAX_SIZE)
        {
            // We keep removing elements from cache until we get enough space to add the element
            remove_cache_element();
        }
        cache_element *element = (cache_element *)malloc(sizeof(cache_element)); // Allocating memory for the new cache element
        element->data = (char *)malloc(size + 1);                                // Allocating memory for the response to be stored in the cache element
        strcpy(element->data, data);
        element->url = (char *)malloc(1 + (strlen(url) * sizeof(char))); // Allocating memory for the request to be stored in the cache element (as a key)
        strcpy(element->url, url);
        element->lru_time_track = time(NULL); // Updating the time_track
        element->next = head;
        element->len = size;
        head = element;
        cache_size += element_size;
        temp_lock_val = pthread_mutex_unlock(&lock);
        printf("Add Cache Lock Unlocked %d\n", temp_lock_val);
        // sem_post(&cache_lock);
        //  free(data);
        //  printf("--\n");
        //  free(url);
        return 1;
    }
    return 0;
}

void remove_cache_element()
{
    // If cache is not empty searches for the node which has the least lru_time_track and deletes it
    cache_element *p;    // Cache_element Pointer (Prev. Pointer)
    cache_element *q;    // Cache_element Pointer (Next Pointer)
    cache_element *temp; // Cache element to remove
    // sem_wait(&cache_lock);
    int temp_lock_val = pthread_mutex_lock(&lock);
    printf("Remove Cache Lock Acquired %d\n", temp_lock_val);
    if (head != NULL)
    { // Cache != empty
        for (q = head, p = head, temp = head; q->next != NULL;
             q = q->next)
        { // Iterate through entire cache and search for oldest time track
            if (((q->next)->lru_time_track) < (temp->lru_time_track))
            {
                temp = q->next;
                p = q;
            }
        }
        if (temp == head)
        {
            head = head->next; /*Handle the base case*/
        }
        else
        {
            p->next = temp->next;
        }
        cache_size = cache_size - (temp->len) - sizeof(cache_element) -
                     strlen(temp->url) - 1; // updating the cache size
        free(temp->data);
        free(temp->url); // Free the removed element
        free(temp);
    }
    // sem_post(&cache_lock);
    temp_lock_val = pthread_mutex_unlock(&lock);
    printf("Remove Cache Lock Unlocked %d\n", temp_lock_val);
}