#pragma once
#include <ctime>

struct cache_element
{
    char *data;
    int len;
    char *url;
    time_t lru_time_track;
};

class CacheStrategy
{
public:
    virtual ~CacheStrategy() = default;
    virtual cache_element *find(const char *url) = 0;
    virtual void add(const char *url, const char *data, int len) = 0;
    virtual void remove(const char *url) = 0;
};