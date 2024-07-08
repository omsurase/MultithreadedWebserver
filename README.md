# Multithreaded HTTP Proxy Server

This project implements a multithreaded HTTP proxy server in C++. It features two server implementations (ThreadPool and Semaphore-based) and two caching strategies (LRU and LFU).

## Features

- Multithreaded server implementations:
  - ThreadPool-based server
  - Semaphore-based server
- Caching strategies:
  - Least Recently Used (LRU) cache
  - Least Frequently Used (LFU) cache
- Factory design pattern for server and cache creation
- Configurable number of threads and cache size

## Project Structure

The project consists of several C++ files implementing different components:

- `main.cpp`: Entry point of the application
- `ServerFactory.cpp/hpp`: Factory for creating server and cache instances
- `HTTPServer.hpp`: Base class for HTTP server implementations
- `ThreadPoolServer.cpp/hpp`: ThreadPool-based server implementation
- `SemaphoreServer.cpp/hpp`: Semaphore-based server implementation
- `CacheStrategy.hpp`: Base class for cache strategies
- `LRUCache.cpp/hpp`: Least Recently Used cache implementation
- `LFUCache.cpp/hpp`: Least Frequently Used cache implementation
- `ThreadPool.cpp/hpp`: ThreadPool implementation
- Various utility classes and headers

## Building the Project

To build the project, ensure you have a C++ compiler and `make` installed. Then run:

```
    make
```

## Running the Server

To run the server, use the following command format:

```
    ./proxy_server <server_type> <cache_type> <port> [num_threads] [cache_size]
```

Where:

- `<server_type>`: Either "Threadpool" or "Semaphore"
- `<cache_type>`: Either "LRUCache" or "LFUCache"
- `<port>`: The port number to run the server on
- `[num_threads]`: (Optional) Number of threads for the server (default: 25)
- `[cache_size]`: (Optional) Maximum number of elements in the cache (default: 100)

Example:

```
    ./proxy_server Threadpool LRUCache 8080 30 100
```

This command starts a ThreadPool-based server with LRU caching on port 8080, using 30 threads and a cache size of 10000 elements.

## Implementation Details

The server uses a factory pattern to create the appropriate server and cache instances based on command-line arguments. It supports two types of server implementations:

1. ThreadPool-based server: Uses a pool of worker threads to handle incoming connections.
2. Semaphore-based server: Uses semaphores to control concurrent access to server resources.

Two caching strategies are implemented:

1. LRU (Least Recently Used): Discards the least recently used items first when the cache is full.
2. LFU (Least Frequently Used): Discards the least frequently used items first when the cache is full.

## Contributing

Contributions to improve the server's functionality or performance are welcome. Please submit pull requests or open issues to discuss proposed changes.
