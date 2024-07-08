#pragma once
#include "HTTPServer.hpp"
#include <semaphore.h>

class SemaphoreServer : public HTTPServer
{
public:
    SemaphoreServer(std::unique_ptr<CacheStrategy> cache, int numThreads = 25);
    void start(int port) override;

private:
    int serverSocket;
    int numThreads;
    sem_t thread_semaphore;
    static void *staticWorkerThread(void *arg);
    void *workerThread();
    void datafromclient(int clientSocket);
};