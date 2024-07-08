#include "ThreadPoolServer.hpp"
#include "ProxyUtils.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
extern "C"
{
#include "proxy_parse.h"
}
#include <iostream>
// HERE after every response is sent to client we close the connection . objective here to make sure other clients get turn too
ThreadPoolServer::ThreadPoolServer(std::unique_ptr<CacheStrategy> cache, int numThreads)
    : HTTPServer(std::move(cache)), pool(numThreads) {}

void ThreadPoolServer::start(int port)
{
    // std::cout << "inside start\n";
    struct sockaddr cli_addr;
    int serverSocket = createServerSocket(port);
    pool.waitForThreads();
    int clilen = sizeof(struct sockaddr);
    while (true)
    {
        int clientSocket = accept(serverSocket, &cli_addr, (socklen_t *)&clilen);
        if (clientSocket < 0)
        {
            fprintf(stderr, "ERROR! On Accepting Request ! i.e requests limit crossed \n");
            continue;
        }
        printf("New client connection accepted\n\n");
        pool.enqueue([this, clientSocket]
                     { handleClient(clientSocket); });
    }
}

void ThreadPoolServer::handleClient(int clientSocket)
{
    // Implement client handling logic here
    // Use the cache member to interact with the cache
    // Use ProxyUtils functions for common operations
    // std::cout << "inside handle client\n";
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
    // std::cout << "inside handle client\n";
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
        writeToClient(newsockfd, iServerfd, *(this->cache), url.c_str());
        close(iServerfd);
    }

    ParsedRequest_destroy(req);
    free(request_message);
    close(newsockfd);
}