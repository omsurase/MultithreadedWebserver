#include "./LRUCache.hpp"
#include <cstring>

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
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto it = cache_map.find(url);
    if (it == cache_map.end())
        return nullptr;

    moveToFront(it->second);
    (*it->second)->lru_time_track = time(nullptr);
    return *it->second;
}

void LRUCache::add(const char *url, const char *data, int len)
{
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto it = cache_map.find(url);
    if (it != cache_map.end())
    {
        moveToFront(it->second);
        return;
    }

    if (cache_list.size() >= capacity)
    {
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
}

void LRUCache::remove(const char *url)
{
    std::lock_guard<std::mutex> lock(cache_mutex);
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

void LRUCache::moveToFront(std::list<cache_element *>::iterator it)
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