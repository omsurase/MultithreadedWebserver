#include "./LFUCache.hpp"
#include <cstring>
#include <algorithm>

LFUCache::LFUCache(size_t cap) : capacity(cap)
{
    frequency_list.push_back({0, {}});
}

LFUCache::~LFUCache()
{
    for (auto &node : frequency_list)
    {
        for (auto elem : node.elements)
        {
            free(elem->data);
            free(elem->url);
            delete elem;
        }
    }
}

cache_element *LFUCache::find(const char *url)
{
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto it = cache_map.find(url);
    if (it == cache_map.end())
    {
        return nullptr;
    }

    auto [node, element_it] = it->second;
    incrementFrequency(url);
    (*element_it)->lru_time_track = time(nullptr);
    return *element_it;
}

void LFUCache::add(const char *url, const char *data, int len)
{
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache_map.find(url) != cache_map.end())
    {
        incrementFrequency(url);
        return;
    }

    if (cache_map.size() >= capacity)
    {
        evict();
    }

    cache_element *new_element = new cache_element;
    new_element->url = strdup(url);
    new_element->data = (char *)malloc(len);
    memcpy(new_element->data, data, len);
    new_element->len = len;
    new_element->lru_time_track = time(nullptr);

    auto &least_frequent = frequency_list.front();
    least_frequent.elements.push_front(new_element);
    cache_map[url] = {&least_frequent, least_frequent.elements.begin()};
}

void LFUCache::remove(const char *url)
{
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto it = cache_map.find(url);
    if (it == cache_map.end())
    {
        return;
    }

    auto [node, element_it] = it->second;
    cache_element *elem = *element_it;
    node->elements.erase(element_it);
    cache_map.erase(it);

    free(elem->data);
    free(elem->url);
    delete elem;
}

void LFUCache::incrementFrequency(const std::string &url)
{
    auto [current_node, element_it] = cache_map[url];
    auto next_frequency = current_node->frequency + 1;
    auto next_it = std::next(std::find_if(frequency_list.begin(), frequency_list.end(),
                                          [next_frequency](const FrequencyNode &node)
                                          { return node.frequency >= next_frequency; }));

    if (next_it == frequency_list.end() || next_it->frequency != next_frequency)
    {
        next_it = frequency_list.insert(next_it, {next_frequency, {}});
    }

    next_it->elements.splice(next_it->elements.begin(), current_node->elements, element_it);
    cache_map[url] = {&*next_it, next_it->elements.begin()};

    if (current_node->elements.empty() && current_node != &frequency_list.front())
    {
        frequency_list.erase(std::find_if(frequency_list.begin(), frequency_list.end(),
                                          [current_node](const FrequencyNode &node)
                                          { return &node == current_node; }));
    }
}

void LFUCache::evict()
{
    auto least_frequent = frequency_list.begin();
    while (least_frequent->elements.empty())
    {
        ++least_frequent;
    }

    auto victim = least_frequent->elements.back();
    cache_map.erase(victim->url);
    least_frequent->elements.pop_back();

    free(victim->data);
    free(victim->url);
    delete victim;

    if (least_frequent->elements.empty() && least_frequent != frequency_list.begin())
    {
        frequency_list.erase(least_frequent);
    }
}