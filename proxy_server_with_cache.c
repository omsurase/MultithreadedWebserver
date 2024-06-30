#include "proxy_parse.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct cache_element cache_element;

// element inside LRU cache.
struct cache_element
{
    char *data;            // data present in response of a respective requests.
    int len;               // len of data (no of bytes).
    char *url;             // url from where response was requested (used for searching).
    time_t lru_time_track; // time when element was inserted in cache.
    cache_element *next;   // pointer to next element in cache.
};

cache_element *find(char *url);                        // find a element in cache.
int add_cache_element(char *data, int len, char *url); // add an element to cache.
void remove_cache_element();                           // remove an element from cache.

int main(int argc, char *argv[])
{
    return 0;
}