#include "SemaphoreServer.hpp"
#include "ProxyUtils.hpp"
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdexcept>

extern "C"
{
#include "proxy_parse.h"
}

volatile sig_atomic_t running = 1;

void sigint_handler(int signum)
{
    running = 0;
}

SemaphoreServer::SemaphoreServer(std::unique_ptr<CacheStrategy> cache, int numThreads)
    : HTTPServer(std::move(cache)), numThreads(numThreads)
{
    sem_init(&thread_semaphore, 0, numThreads);
}

void *SemaphoreServer::staticWorkerThread(void *arg)
{
    SemaphoreServer *server = static_cast<SemaphoreServer *>(arg);
    return server->workerThread();
}
void *SemaphoreServer::workerThread()
{
    while (running)
    {
        sem_wait(&thread_semaphore);

        if (!running)
        {
            sem_post(&thread_semaphore);
            break;
        }

        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket < 0)
        {
            // if (errno == EINTR)
            // {
            //     sem_post(&thread_semaphore);
            //     continue;
            // }
            // perror("Error accepting connection");
            // sem_post(&thread_semaphore);
            // continue;
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                sem_post(&thread_semaphore);
                continue;
            }
            perror("Error accepting connection");
            sem_post(&thread_semaphore);
            continue;
        }

        printf("New client connection accepted\n\n");
        this->datafromclient(clientSocket);
        close(clientSocket);
        sem_post(&thread_semaphore);
    }

    return NULL;
}

// void SemaphoreServer::start(int port)
// {
//     int serverSocket = createServerSocket(port);
//     if (serverSocket < 0)
//     {
//         throw std::runtime_error("Failed to create server socket");
//     }
//     signal(SIGINT, sigint_handler);

//     pthread_t worker_threads[MAX_THREADS];
//     for (int i = 0; i < MAX_THREADS; i++)
//     {
//         pthread_create(&worker_threads[i], NULL, staticWorkerThread, this);
//     }

//     for (int i = 0; i < MAX_THREADS; i++)
//     {
//         pthread_join(worker_threads[i], NULL);
//     }

//     sem_destroy(&thread_semaphore);
//     close(serverSocket);
// }

void SemaphoreServer::start(int port)
{
    serverSocket = createServerSocket(port);
    if (serverSocket < 0)
    {
        throw std::runtime_error("Failed to create server socket");
    }

    signal(SIGINT, sigint_handler);

    pthread_t worker_threads[numThreads];
    for (int i = 0; i < numThreads; i++)
    {
        pthread_create(&worker_threads[i], NULL, staticWorkerThread, this);
    }

    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(worker_threads[i], NULL);
    }

    sem_destroy(&thread_semaphore);
    close(serverSocket);
}

void SemaphoreServer::datafromclient(int clientSocket)
{
    int MAX_BUFFER_SIZE = 5000;
    char buf[MAX_BUFFER_SIZE];
    int newsockfd = clientSocket;
    char *request_message; // Get message from URL
    request_message = (char *)malloc(MAX_BUFFER_SIZE);
    if (request_message == NULL)
    {
        fprintf(stderr, " Error in memory allocation ! \n");
        exit(1);
    }

    request_message[0] = '\0';
    int total_recieved_bits = 0;

    while (strstr(request_message, "\r\n\r\n") == NULL)
    {
        int recvd = recv(newsockfd, buf, MAX_BUFFER_SIZE, 0);

        if (recvd < 0)
        {
            fprintf(stderr, " Error while receiving ! \n");
            exit(1);
        }
        else if (recvd == 0)
        {
            break;
        }
        else
        {
            total_recieved_bits += recvd;

            if (total_recieved_bits > MAX_BUFFER_SIZE)
            {
                MAX_BUFFER_SIZE *= 2;
                request_message = (char *)realloc(request_message, MAX_BUFFER_SIZE);
                if (request_message == NULL)
                {
                    fprintf(stderr, " Error in memory re-allocation ! \n");
                    exit(1);
                }
            }
        }

        strncat(request_message, buf, recvd);
    }

    struct ParsedRequest *req;
    req = ParsedRequest_create();

    if (ParsedRequest_parse(req, request_message, strlen(request_message)) < 0)
    {
        fprintf(stderr, "Error in request message. Request message: %s\n", request_message);
        exit(0);
    }

    printf("Received request for URL: %s%s\n", req->host, req->path);

    if (req->port == NULL)
        req->port = (char *)"80";

    char *browser_req = convertRequestToString(req);
    std::string url(req->host);
    url += req->path;

    cache_element *cached_response = cache->find(url.c_str());

    if (cached_response != nullptr)
    {
        printf("Cache hit: Response found in cache for URL: %s\n", url.c_str());
        printf("Cache element details:\n");
        printf("  URL: %s\n", cached_response->url);
        printf("  Length: %d bytes\n", cached_response->len);
        printf("  Last accessed: %s", ctime(&cached_response->lru_time_track));
        writeToClientSocket(cached_response->data, newsockfd, cached_response->len);
    }
    else
    {
        printf("Cache miss: Response not found in cache for URL: %s\n", url.c_str());
        int iServerfd = createServerSocket(req->host, req->port);
        printf("Sending request to remote server: %s\n", req->host);
        writeToServerSocket(browser_req, iServerfd, strlen(browser_req));
        writeToClient(newsockfd, iServerfd, *(cache), url.c_str());
        close(iServerfd);
    }

    ParsedRequest_destroy(req);
    free(request_message);
    close(newsockfd);
}

// void *SemaphoreServer::workerThread(void *arg)
// {
//     int serverSocket = *((int *)arg);

//     while (running)
//     {
//         sem_wait(&thread_semaphore);

//         if (!running)
//         {
//             sem_post(&thread_semaphore);
//             break;
//         }

//         int clientSocket = accept(serverSocket, NULL, NULL);
//         if (clientSocket < 0)
//         {
//             if (errno == EINTR)
//             {
//                 sem_post(&thread_semaphore);
//                 continue;
//             }
//             perror("Error accepting connection");
//             sem_post(&thread_semaphore);
//             continue;
//         }

//         printf("New client connection accepted\n");
//         this->datafromclient(clientSocket);
//         close(clientSocket);
//         sem_post(&thread_semaphore);
//     }

//     return NULL;
// }