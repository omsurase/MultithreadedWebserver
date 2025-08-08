#pragma once
#include "core/HTTPServer.hpp"
#include "core/ThreadPool.hpp"

class ThreadPoolServer : public HTTPServer
{
public:
    ThreadPoolServer(std::unique_ptr<CacheStrategy> cache, int numThreads = 25);
    void start(int port) override;

private:
    ThreadPool pool;
    void handleClient(int clientSocket);
};