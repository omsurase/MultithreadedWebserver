# Multithreaded HTTP Proxy Server

This project implements a multithreaded HTTP proxy server in C++. It features two server implementations (ThreadPool and Semaphore-based) and two caching strategies (LRU and LFU).

## Introduction

This proxy server acts as an intermediary for requests from clients seeking resources from other servers. It incorporates several advanced concepts:

- **Multithreading**: Allows the server to handle multiple client requests concurrently.
- **ThreadPool**: A pool of worker threads that can be reused to perform tasks, reducing the overhead of thread creation.
- **Semaphores**: Used for synchronization between threads, controlling access to shared resources.
- **Caching**: Stores frequently or recently accessed data to improve response times and reduce network traffic.
- **LRU (Least Recently Used)**: A caching algorithm that discards the least recently used items first.
- **LFU (Least Frequently Used)**: A caching algorithm that discards the least frequently used items first.
- **Factory Design Pattern**: Used to create different types of servers and caches based on runtime configuration.

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

- [`main.cpp`](main.cpp): Entry point of the application
- [`ServerFactory.cpp/hpp`](ServerFactory.hpp): Factory for creating server and cache instances
- [`HTTPServer.hpp`](HTTPServer.hpp): Base class for HTTP server implementations
- [`ThreadPoolServer.cpp/hpp`](ThreadPoolServer.hpp): ThreadPool-based server implementation
- [`SemaphoreServer.cpp/hpp`](SemaphoreServer.hpp): Semaphore-based server implementation
- [`CacheStrategy.hpp`](CacheStrategy.hpp): Base class for cache strategies
- [`LRUCache.cpp/hpp`](LRUCache.hpp): Least Recently Used cache implementation
- [`LFUCache.cpp/hpp`](LFUCache.hpp): Least Frequently Used cache implementation
- [`ThreadPool.cpp/hpp`](ThreadPool.hpp): ThreadPool implementation
- [`Proxy_utils.cpp/hpp`](ThreadPool.hpp)Various utility classes and headers

## Building the Project

To build the project, ensure you have a Linux machine with C++ compiler and `make` installed. Then run:

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

## Testing the Proxy Server

Once the server is running, you can test it using Postman or any HTTP client. Here's how to use Postman:

1. Open Postman and create a new request.
2. Set the request type to GET (or the appropriate HTTP method).
3. Enter the URL you want to access, but replace the host and port with your proxy server's address. For example:

```
  http://localhost:8080/http://example.com
```

This sends a request to `http://example.com` through your proxy server running on `localhost:8080`. 4. Send the request and observe the response.

The proxy server should forward your request to the target server and return the response. You can verify the proxy's functionality by checking the response headers or timing multiple requests to see if caching is working.

## Implementation Details

The server uses a factory pattern to create the appropriate server and cache instances based on command-line arguments. It supports two types of server implementations:

1. ThreadPool-based server: Uses a pool of worker threads to handle incoming connections.
2. Semaphore-based server: Uses semaphores to control concurrent access to server resources.

Two caching strategies are implemented:

1. LRU (Least Recently Used): Discards the least recently used items first when the cache is full.
2. LFU (Least Frequently Used): Discards the least frequently used items first when the cache is full.

## Contributing

Contributions to improve the server's functionality or performance are welcome. Please submit pull requests or open issues to discuss proposed changes.
