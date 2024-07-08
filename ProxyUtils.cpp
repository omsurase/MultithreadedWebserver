#include "ProxyUtils.hpp"
extern "C"
{
#include "proxy_parse.h"
}
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include "./CacheStrategy.hpp"

int createServerSocket(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return -1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Error binding socket");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 100) < 0)
    {
        perror("Error listening on socket");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

// Implement other functions (convertRequestToString, createServerSocket, writeToServerSocket, writeToClientSocket, writeToClient) here
// These implementations should be based on the original code provided in the question

char *convertRequestToString(struct ParsedRequest *req)
{
    // Set headers
    ParsedHeader_set(req, "Host", req->host);
    ParsedHeader_set(req, "Connection", "close");

    int headersLen = ParsedHeader_headersLen(req);
    char *headersBuf = (char *)malloc(headersLen + 1);
    if (!headersBuf)
    {
        perror("Failed to allocate memory for headers");
        return nullptr;
    }

    ParsedRequest_unparse_headers(req, headersBuf, headersLen);
    headersBuf[headersLen] = '\0';

    int requestSize = strlen(req->method) + strlen(req->path) + strlen(req->version) + headersLen + 4;
    char *serverReq = (char *)malloc(requestSize + 1);
    if (!serverReq)
    {
        perror("Failed to allocate memory for server request");
        free(headersBuf);
        return nullptr;
    }

    snprintf(serverReq, requestSize + 1, "%s %s %s\r\n%s", req->method, req->path, req->version, headersBuf);

    free(headersBuf);
    return serverReq;
}

int createServerSocket(const char *address, const char *port)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(address, port, &hints, &res) != 0)
    {
        perror("getaddrinfo failed");
        return -1;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0)
    {
        perror("Failed to create socket");
        freeaddrinfo(res);
        return -1;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("Failed to connect");
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    return sockfd;
}

void writeToSocket(const char *buff, int sockfd, int length)
{
    int totalSent = 0;
    while (totalSent < length)
    {
        int sent = send(sockfd, buff + totalSent, length - totalSent, 0);
        if (sent < 0)
        {
            perror("Error in sending data");
            return;
        }
        totalSent += sent;
    }
}

void writeToServerSocket(const char *buff, int sockfd, int length)
{
    writeToSocket(buff, sockfd, length);
}

void writeToClientSocket(const char *buff, int sockfd, int length)
{
    writeToSocket(buff, sockfd, length);
}

// void writeToClient(int clientfd, int serverfd, CacheStrategy &cache, const char *url)
// {
//     char buffer[4096];
//     ssize_t bytesRead;
//     std::string response;

//     while ((bytesRead = recv(serverfd, buffer, sizeof(buffer), 0)) > 0)
//     {
//         response.append(buffer, bytesRead);
//         writeToClientSocket(buffer, clientfd, bytesRead);
//     }

//     if (bytesRead < 0)
//     {
//         perror("Error receiving from server");
//     }
//     else
//     {
//         cache.add(url, response.c_str(), response.size());
//     }
// }

void writeToClient(int clientfd, int serverfd, CacheStrategy &cache, const char *url)
{
    char buffer[4096];
    ssize_t bytesRead;
    std::string response;
    while ((bytesRead = recv(serverfd, buffer, sizeof(buffer), 0)) > 0)
    {
        response.append(buffer, bytesRead);
        writeToClientSocket(buffer, clientfd, bytesRead);
    }
    if (bytesRead < 0)
    {
        perror("Error receiving from server");
    }
    else
    {
        printf("Adding new element to cache:\n");
        printf("  URL: %s\n", url);
        printf("  Response size: %zu bytes\n", response.size());

        // Extract and print content type if available
        size_t contentTypePos = response.find("Content-Type:");
        if (contentTypePos != std::string::npos)
        {
            size_t endOfLine = response.find("\r\n", contentTypePos);
            if (endOfLine != std::string::npos)
            {
                std::string contentType = response.substr(contentTypePos + 14, endOfLine - (contentTypePos + 14));
                printf("  Content-Type: %s\n", contentType.c_str());
            }
        }

        // Print current time
        time_t now = time(nullptr);
        printf("  Cached at: %s", ctime(&now));

        cache.add(url, response.c_str(), response.size());

        printf("Element successfully added to cache.\n\n");
    }
}