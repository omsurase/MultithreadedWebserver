#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <netdb.h>
#include <atomic>
#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <unordered_map>
#include <list>

#include "proxy_parse.h"

using namespace std;

struct cache_element
{
    char *data;
    int len;
    char *url;
    time_t lru_time_track;
};

class LRUCache
{
public:
    LRUCache(size_t capacity);
    ~LRUCache();

    cache_element *find(const char *url);
    void add(const char *url, const char *data, int len);
    void remove(const char *url);

private:
    size_t capacity;
    list<cache_element *> cache_list;
    unordered_map<string, list<cache_element *>::iterator> cache_map;
    mutex cache_mutex;

    void moveToFront(list<cache_element *>::iterator it);
    void evict();
};

LRUCache::LRUCache(size_t cap) : capacity(cap) {}

LRUCache::~LRUCache()
{
    for (auto elem : cache_list)
    {
        free(elem->data);
        free(elem->url);
        delete elem;
    }
}

cache_element *LRUCache::find(const char *url)
{
    lock_guard<mutex> lock(cache_mutex);
    auto it = cache_map.find(url);
    if (it == cache_map.end())
        return nullptr;

    moveToFront(it->second);
    (*it->second)->lru_time_track = time(nullptr);
    return *it->second;
}

void LRUCache::add(const char *url, const char *data, int len)
{
    lock_guard<mutex> lock(cache_mutex);
    auto it = cache_map.find(url);
    if (it != cache_map.end())
    {
        moveToFront(it->second);
        printf("Updating existing cache entry for URL: %s\n", url);
        return;
    }

    if (cache_list.size() >= capacity)
    {
        printf("Cache full, evicting least recently used entry\n");
        evict();
    }

    cache_element *new_element = new cache_element;
    new_element->url = strdup(url);
    new_element->data = (char *)malloc(len);
    memcpy(new_element->data, data, len);
    new_element->len = len;
    new_element->lru_time_track = time(nullptr);

    cache_list.push_front(new_element);
    cache_map[url] = cache_list.begin();
    printf("Stored new response in cache for URL: %s\n", url);
}

void LRUCache::remove(const char *url)
{
    lock_guard<mutex> lock(cache_mutex);
    auto it = cache_map.find(url);
    if (it == cache_map.end())
        return;

    cache_element *elem = *it->second;
    cache_list.erase(it->second);
    cache_map.erase(it);

    free(elem->data);
    free(elem->url);
    delete elem;
}

void LRUCache::moveToFront(list<cache_element *>::iterator it)
{
    cache_list.splice(cache_list.begin(), cache_list, it);
}

void LRUCache::evict()
{
    cache_element *elem = cache_list.back();
    cache_map.erase(elem->url);
    cache_list.pop_back();

    free(elem->data);
    free(elem->url);
    delete elem;
}

class ThreadPool
{
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    void enqueue(std::function<void()> task);
    void waitForThreads(); // New method

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::atomic<size_t> readyThreads;        // New variable to track ready threads
    std::condition_variable allThreadsReady; // New condition variable
};

ThreadPool::ThreadPool(size_t numThreads) : stop(false), readyThreads(0)
{
    for (size_t i = 0; i < numThreads; ++i)
    {
        workers.emplace_back([this]
                             {
            {
                std::unique_lock<std::mutex> lock(this->queueMutex);
                readyThreads++;
                allThreadsReady.notify_one();
            }
            for(;;) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    if(this->stop && this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }

                task();
            } });
    }
}

ThreadPool::~ThreadPool()
{
    stop = true;
    condition.notify_all();
    for (std::thread &worker : workers)
        worker.join();
}

void ThreadPool::enqueue(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.emplace(task);
    }
    condition.notify_one();
}

char *convert_Request_to_string(struct ParsedRequest *req)
{
    /* Set headers */
    ParsedHeader_set(req, "Host", req->host);
    ParsedHeader_set(req, "Connection", "close");

    int iHeadersLen = ParsedHeader_headersLen(req);

    char *headersBuf;

    headersBuf = (char *)malloc(iHeadersLen + 1);

    if (headersBuf == NULL)
    {
        fprintf(stderr, " Error in memory allocation  of headersBuffer ! \n");
        exit(1);
    }

    ParsedRequest_unparse_headers(req, headersBuf, iHeadersLen);
    headersBuf[iHeadersLen] = '\0';

    int request_size = strlen(req->method) + strlen(req->path) + strlen(req->version) + iHeadersLen + 4;

    char *serverReq;

    serverReq = (char *)malloc(request_size + 1);

    if (serverReq == NULL)
    {
        fprintf(stderr, " Error in memory allocation for serverrequest ! \n");
        exit(1);
    }

    serverReq[0] = '\0';
    strcpy(serverReq, req->method);
    strcat(serverReq, " ");
    strcat(serverReq, req->path);
    strcat(serverReq, " ");
    strcat(serverReq, req->version);
    strcat(serverReq, "\r\n");
    strcat(serverReq, headersBuf);

    free(headersBuf);

    return serverReq;
}

int createserverSocket(char *pcAddress, char *pcPort)
{
    struct addrinfo ahints;
    struct addrinfo *paRes;

    int iSockfd;

    /* Get address information for stream socket on input port */
    memset(&ahints, 0, sizeof(ahints));
    ahints.ai_family = AF_UNSPEC;
    ahints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(pcAddress, pcPort, &ahints, &paRes) != 0)
    {
        fprintf(stderr, " Error in server address format ! \n");
        exit(1);
    }

    /* Create and connect */
    if ((iSockfd = socket(paRes->ai_family, paRes->ai_socktype, paRes->ai_protocol)) < 0)
    {
        fprintf(stderr, " Error in creating socket to server ! \n");
        exit(1);
    }
    if (connect(iSockfd, paRes->ai_addr, paRes->ai_addrlen) < 0)
    {
        fprintf(stderr, " Error in connecting to server ! \n");
        exit(1);
    }

    /* Free paRes, which was dynamically allocated by getaddrinfo */
    freeaddrinfo(paRes);

    return iSockfd;
}

void writeToserverSocket(const char *buff_to_server, int sockfd, int buff_length)
{
    string temp;
    temp.append(buff_to_server);

    int totalsent = 0;
    int senteach;

    while (totalsent < buff_length)
    {
        if ((senteach = send(sockfd, (void *)(buff_to_server + totalsent), buff_length - totalsent, 0)) < 0)
        {
            fprintf(stderr, " Error in sending to server ! \n");
            exit(1);
        }
        totalsent += senteach;
    }
}

void writeToclientSocket(const char *buff_to_client, int sockfd, int buff_length)
{
    string temp;
    temp.append(buff_to_client);

    int totalsent = 0;
    int senteach;

    while (totalsent < buff_length)
    {
        if ((senteach = send(sockfd, (void *)(buff_to_client + totalsent), buff_length - totalsent, 0)) < 0)
        {
            fprintf(stderr, " Error in sending to client ! \n");
            exit(1);
        }
        totalsent += senteach;
    }
}

void writeToClient(int Clientfd, int Serverfd, LRUCache &cache, const char *url)
{
    int MAX_BUF_SIZE = 5000;
    int iRecv;
    char buf[MAX_BUF_SIZE];

    string response;
    while ((iRecv = recv(Serverfd, buf, MAX_BUF_SIZE, 0)) > 0)
    {
        response.append(buf, iRecv);
        writeToclientSocket(buf, Clientfd, iRecv);
        memset(buf, 0, sizeof buf);
    }

    if (iRecv < 0)
    {
        fprintf(stderr, " Error while receiving from server ! \n");
        exit(1);
    }

    cache.add(url, response.c_str(), response.size());
    printf("Response received and stored in cache for URL: %s\n", url);
}

void ThreadPool::waitForThreads()
{
    std::unique_lock<std::mutex> lock(queueMutex);
    allThreadsReady.wait(lock, [this]
                         { return readyThreads == workers.size(); });
}

void *datafromclient(void *sockid)
{
    int MAX_BUFFER_SIZE = 5000;
    char buf[MAX_BUFFER_SIZE];

    int newsockfd = *((int *)sockid);

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

    char *browser_req = convert_Request_to_string(req);
    string url(req->host);
    url += req->path;

    static LRUCache cache(100); // Define the cache with a size of 100
    cache_element *cached_response = cache.find(url.c_str());

    if (cached_response != nullptr)
    {
        printf("Cache hit: Response found in cache for URL: %s\n", url.c_str());
        writeToclientSocket(cached_response->data, newsockfd, cached_response->len);
    }
    else
    {
        printf("Cache miss: Response not found in cache for URL: %s\n", url.c_str());
        int iServerfd = createserverSocket(req->host, req->port);
        printf("Sending request to remote server: %s\n", req->host);
        writeToserverSocket(browser_req, iServerfd, strlen(browser_req));
        writeToClient(newsockfd, iServerfd, cache, url.c_str());
        close(iServerfd);
    }

    ParsedRequest_destroy(req);
    free(request_message);
    close(newsockfd);

    int y = 3;
    int *p = &y;
    return p;
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr;
    struct sockaddr cli_addr;

    if (argc < 2)
    {
        fprintf(stderr, "SORRY! Provide A Port ! \n");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        fprintf(stderr, "SORRY! Cannot create a socket ! \n");
        return 1;
    }

    memset(&serv_addr, 0, sizeof serv_addr);

    int portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    int binded = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if (binded < 0)
    {
        fprintf(stderr, "Error on binding! \n");
        return 1;
    }

    listen(sockfd, 1000);
    printf("Proxy server listening on port %d\n", portno);

    int clilen = sizeof(struct sockaddr);
    ThreadPool pool(250);
    pool.waitForThreads();

    while (1)
    {
        newsockfd = accept(sockfd, &cli_addr, (socklen_t *)&clilen);

        if (newsockfd < 0)
        {
            fprintf(stderr, "ERROR! On Accepting Request ! i.e requests limit crossed \n");
            continue;
        }
        printf("New client connection accepted\n");
        pool.enqueue([newsockfd]
                     {
            datafromclient((void *)&newsockfd);
            close(newsockfd); });
    }

    close(sockfd);

    return 0;
}
