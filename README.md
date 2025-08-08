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

After reorganization, the project uses a conventional `src/` + `include/` layout:

- `src/`
  - `main.cpp`
  - `core/`
    - `ServerFactory.cpp`
    - `ThreadPool.cpp`
  - `servers/`
    - `ThreadPoolServer.cpp`
    - `SemaphoreServer.cpp`
  - `cache/`
    - `LRUCache.cpp`
    - `LFUCache.cpp`
  - `utils/`
    - `ProxyUtils.cpp`
  - `c/`
    - `proxy_parse.c`

- `include/`
  - `core/`
    - `HTTPServer.hpp`
    - `ServerFactory.hpp`
    - `ThreadPool.hpp`
  - `servers/`
    - `ThreadPoolServer.hpp`
    - `SemaphoreServer.hpp`
  - `cache/`
    - `CacheStrategy.hpp`
    - `LRUCache.hpp`
    - `LFUCache.hpp`
  - `utils/`
    - `ProxyUtils.hpp`
  - `c/`
    - `proxy_parse.h`

Legacy examples are under `examples/v1/`.

## Build and run with Docker

You can build and run the project entirely using Docker (no local toolchain required):

```powershell
docker build -t proxy-server .

# Example: Threadpool + LRUCache on port 8080
docker run --rm -p 8080:8080 proxy-server Threadpool LRUCache 8080 25 100

# Example: Semaphore + LFUCache on port 9090
docker run --rm -p 9090:9090 proxy-server Semaphore LFUCache 9090 25 100
```

### Start, test, stop (PowerShell)

Run the container in the background with a name, test a few URLs, then stop it.

```powershell
# Threadpool + LRUCache
$PORT = 8080
$NAME = "proxy-$PORT"
docker run --rm -d --name $NAME -p $PORT:$PORT proxy-server Threadpool LRUCache $PORT 25 100
Start-Sleep -Seconds 2

# Test a few requests (repeat a URL to see cache hit in logs)
curl "http://localhost:$PORT/http://example.com"
curl "http://localhost:$PORT/http://www.google.com"
curl "http://localhost:$PORT/http://www.google.com"  # cache hit expected

# Stop and remove the container
docker stop $NAME
```

Try another configuration (Semaphore + LFUCache) on a different port:

```powershell
$PORT = 9090
$NAME = "proxy-$PORT"
docker run --rm -d --name $NAME -p $PORT:$PORT proxy-server Semaphore LFUCache $PORT 50 200
Start-Sleep -Seconds 2

curl "http://localhost:$PORT/http://example.com"
curl "http://localhost:$PORT/http://www.wikipedia.org"
curl "http://localhost:$PORT/http://www.wikipedia.org"  # cache hit expected

docker stop $NAME
```

### Arguments

`./proxy_server <server_type> <cache_type> <port> [num_threads] [cache_size]`

- `server_type`: `Threadpool` or `Semaphore`
- `cache_type`: `LRUCache` or `LFUCache`
- `port`: host/container port to bind
- `num_threads`: optional, default 25
- `cache_size`: optional, default 100

## Test the proxy

- **cURL**:
  - `curl "http://localhost:8080/http://example.com"`
- **Browser/Postman**:
  - Request URL: `http://localhost:8080/http://example.com`
  - Repeat the same URL to observe cache hits in the container logs.

## Local build (optional)

If you are on Linux and prefer local builds:

```bash
make clean && make -j
./proxy_server Threadpool LRUCache 8080 25 100
```

## Contributing

Contributions to improve the server's functionality or performance are welcome. Please submit pull requests or open issues to discuss proposed changes.
