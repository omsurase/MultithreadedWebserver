// ServerFactory.hpp
#pragma once
#include "./HTTPServer.hpp"
#include "./CacheStrategy.hpp"

class ServerFactory
{
public:
    static std::unique_ptr<HTTPServer> createHTTPServer(const std::string &serverType, const std::string &cacheType, int numThreads = 25, int cacheSize = 100);

private:
    static std::unique_ptr<CacheStrategy> createCacheStrategy(const std::string &cacheType, int cacheSize);
};