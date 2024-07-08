#pragma once
#include "./CacheStrategy.hpp"
#include <list>
#include <unordered_map>
#include <string>
#include <mutex>

class LFUCache : public CacheStrategy
{
public:
    LFUCache(size_t capacity);
    ~LFUCache() override;

    cache_element *find(const char *url) override;
    void add(const char *url, const char *data, int len) override;
    void remove(const char *url) override;

private:
    struct FrequencyNode
    {
        int frequency;
        std::list<cache_element *> elements;
    };

    size_t capacity;
    std::unordered_map<std::string, std::pair<FrequencyNode *, std::list<cache_element *>::iterator>> cache_map;
    std::list<FrequencyNode> frequency_list;
    std::mutex cache_mutex;

    void incrementFrequency(const std::string &url);
    void evict();
};