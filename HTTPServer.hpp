#pragma once
#include "./CacheStrategy.hpp"
#include <memory>

class HTTPServer
{
public:
    HTTPServer(std::unique_ptr<CacheStrategy> cache) : cache(std::move(cache)) {}
    virtual ~HTTPServer() = default;

    virtual void start(int port) = 0;

protected:
    std::unique_ptr<CacheStrategy> cache;
};