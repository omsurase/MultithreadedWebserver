# Multithreaded HTTP Proxy Server

A high-performance multithreaded HTTP proxy server implemented in C++ with configurable server architectures and intelligent caching strategies.

## What It Does

This proxy server acts as an intermediary between clients and web servers, forwarding HTTP requests and caching responses for improved performance. It supports multiple concurrent connections and implements intelligent caching to reduce response times for frequently requested content.

## Features

- **Multiple Server Architectures**: ThreadPool-based and Semaphore-based concurrency models
- **Intelligent Caching**: LRU (Least Recently Used) and LFU (Least Frequently Used) cache strategies
- **Runtime Configuration**: Fully configurable via environment variables or `.env` files
- **High Performance**: Optimized for concurrent request handling
- **Docker Ready**: Containerized deployment with Docker Compose

## Quick Start

### Method 1: Using Predefined Default Values

Start the server with default configuration (ThreadPool server, LRU cache, port 8080, 25 threads, cache size 100):

```bash
# Start server with defaults
docker compose up proxy-server

# Start in detached mode
docker compose up -d proxy-server

# View logs
docker compose logs -f proxy-server

# Stop server
docker compose down
```

### Method 2: Using Custom Values with .env File

1. **Create your configuration file:**
```bash
cp docker.env.example .env
```

2. **Edit `.env` with your settings:**

For **development** setup:
```bash
SERVER_TYPE=Threadpool
CACHE_TYPE=LRUCache
PROXY_PORT=8080
THREAD_COUNT=10
CACHE_SIZE=50
CONTAINER_NAME=dev-proxy
```

For **production** setup:
```bash
SERVER_TYPE=Threadpool
CACHE_TYPE=LFUCache
PROXY_PORT=8080
THREAD_COUNT=50
CACHE_SIZE=500
CONTAINER_NAME=prod-proxy
```

For **semaphore-based** setup:
```bash
SERVER_TYPE=Semaphore
CACHE_TYPE=LRUCache
PROXY_PORT=8082
THREAD_COUNT=30
CACHE_SIZE=200
CONTAINER_NAME=semaphore-proxy
```

3. **Start the server:**
```bash
# Start with your .env configuration
docker compose up -d proxy-server

# View logs
docker compose logs -f proxy-server

# Stop server
docker compose down
```

## Configuration Options

| Variable | Options | Default | Description |
|----------|---------|---------|-------------|
| SERVER_TYPE | `Threadpool`, `Semaphore` | `Threadpool` | Server concurrency model |
| CACHE_TYPE | `LRUCache`, `LFUCache` | `LRUCache` | Cache eviction strategy |
| PROXY_PORT | Any valid port | `8080` | Port to run the server on |
| THREAD_COUNT | Positive integer | `25` | Number of threads/semaphores |
| CACHE_SIZE | Positive integer | `100` | Maximum cache entries |
| CONTAINER_NAME | String | `proxy-server` | Docker container name |

## Testing the Proxy Server

### Basic Functionality Test

```bash
# Test basic proxy functionality
curl "http://localhost:8080/http://example.com"
curl "http://localhost:8080/http://httpbin.org/json"

# Test cache hits (repeat same URL)
curl "http://localhost:8080/http://example.com"  # Should be cached
```

### Performance Testing

```bash
# Test concurrent requests
for i in {1..10}; do
  curl "http://localhost:8080/http://httpbin.org/delay/1" &
done
wait

# Test cache performance
for i in {1..5}; do
  curl "http://localhost:8080/http://example.com"
done
```

## Advanced Examples

### Command Line Environment Variables

You can also override settings directly from the command line:

```bash
# Custom configuration with inline environment variables
SERVER_TYPE=Semaphore CACHE_TYPE=LFUCache PROXY_PORT=8082 THREAD_COUNT=30 CACHE_SIZE=150 docker compose up proxy-server

# High performance configuration
SERVER_TYPE=Threadpool CACHE_TYPE=LFUCache PROXY_PORT=8080 THREAD_COUNT=100 CACHE_SIZE=1000 docker compose up proxy-server

# Development configuration
SERVER_TYPE=Threadpool CACHE_TYPE=LRUCache PROXY_PORT=3000 THREAD_COUNT=5 CACHE_SIZE=25 docker compose up proxy-server
```

## Architecture

- **ThreadPool Server**: Uses a pool of worker threads to handle requests concurrently
- **Semaphore Server**: Uses semaphores to control concurrent request processing
- **LRU Cache**: Evicts least recently used items when cache is full
- **LFU Cache**: Evicts least frequently used items when cache is full

## Legacy Support

For backward compatibility, preset configurations are available using Docker Compose profiles:

```bash
# Available legacy profiles
docker compose --profile legacy up                    # All legacy services
docker compose --profile threadpool-lfu up           # ThreadPool + LFU
docker compose --profile semaphore-lru up            # Semaphore + LRU
docker compose --profile high-perf up                # High performance preset
```
