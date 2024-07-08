#include "ServerFactory.hpp"
#include "ThreadPoolServer.hpp"
#include "SemaphoreServer.hpp"
#include "./LRUCache.hpp"
#include "./LFUCache.hpp"

std::unique_ptr<HTTPServer> ServerFactory::createHTTPServer(const std::string &serverType, const std::string &cacheType, int numThreads, int cacheSize)
{
    auto cache = createCacheStrategy(cacheType, cacheSize);
    if (serverType == "Threadpool")
    {
        return std::make_unique<ThreadPoolServer>(std::move(cache));
    }
    else if (serverType == "Semaphore")
    {
        return std::make_unique<SemaphoreServer>(std::move(cache));
    }
    throw std::invalid_argument("Invalid server type");
}

std::unique_ptr<CacheStrategy> ServerFactory::createCacheStrategy(const std::string &cacheType, int cacheSize)
{
    if (cacheType == "LRUCache")
    {
        return std::make_unique<LRUCache>(cacheSize);
    }
    else if (cacheType == "LFUCache")
    {
        return std::make_unique<LFUCache>(cacheSize);
    }
    throw std::invalid_argument("Invalid cache type");
}