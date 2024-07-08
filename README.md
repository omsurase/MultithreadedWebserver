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
