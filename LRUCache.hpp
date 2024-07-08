#pragma once
#include "./CacheStrategy.hpp"
#include <list>
#include <unordered_map>
#include <mutex>

class LRUCache : public CacheStrategy
{
public:
    LRUCache(size_t capacity);
    ~LRUCache() override;

    cache_element *find(const char *url) override;
    void add(const char *url, const char *data, int len) override;
    void remove(const char *url) override;

private:
    size_t capacity;
    std::list<cache_element *> cache_list;
    std::unordered_map<std::string, std::list<cache_element *>::iterator> cache_map;
    std::mutex cache_mutex;

    void moveToFront(std::list<cache_element *>::iterator it);
    void evict();
};