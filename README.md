# Multithreaded HTTP Proxy Server

A high-performance multithreaded HTTP proxy server in C++ with multiple server implementations and caching strategies.

## Features

- **Server Types**: ThreadPool-based and Semaphore-based implementations
- **Caching Strategies**: LRU (Least Recently Used) and LFU (Least Frequently Used)
- **Configurable**: Adjustable thread count and cache size
- **Docker Ready**: Easy deployment with Docker Compose

## Quick Start

### Start Default Server (ThreadPool + LRU Cache)

```bash
docker compose up -d
```

### Start Specific Server Configurations

```bash
# ThreadPool + LFU Cache on port 8081
docker compose --profile threadpool-lfu up -d

# Semaphore + LRU Cache on port 8082
docker compose --profile semaphore-lru up -d

# Semaphore + LFU Cache on port 8083
docker compose --profile semaphore-lfu up -d

# High Performance (50 threads, 500 cache size) on port 8090
docker compose --profile high-perf up -d
```

### Stop Services

```bash
# Stop default service
docker compose down

# Stop specific profiles
docker compose --profile threadpool-lfu down
docker compose --profile semaphore-lru down
docker compose --profile semaphore-lfu down
docker compose --profile high-perf down

# Stop all running services
docker compose down --remove-orphans
```

## Testing the Proxy Server

### Basic Testing

Test the default server (port 8080):

```bash
# Test basic functionality
curl "http://localhost:8080/http://example.com"
curl "http://localhost:8080/http://httpbin.org/json"
curl "http://localhost:8080/http://www.google.com"

# Test cache hits (repeat same URL)
curl "http://localhost:8080/http://example.com"  # Should be cached
curl "http://localhost:8080/http://httpbin.org/json"  # Should be cached
```

### Testing Different Server Configurations

```bash
# Test ThreadPool + LFU (port 8081)
docker compose --profile threadpool-lfu up -d
curl "http://localhost:8081/http://example.com"
curl "http://localhost:8081/http://httpbin.org/headers"
curl "http://localhost:8081/http://example.com"  # Cache hit

# Test Semaphore + LRU (port 8082)
docker compose --profile semaphore-lru up -d
curl "http://localhost:8082/http://www.wikipedia.org"
curl "http://localhost:8082/http://httpbin.org/ip"
curl "http://localhost:8082/http://www.wikipedia.org"  # Cache hit

# Test High Performance (port 8090)
docker compose --profile high-perf up -d
curl "http://localhost:8090/http://example.com"
curl "http://localhost:8090/http://httpbin.org/user-agent"
```

### Performance Testing

```bash
# Test multiple concurrent requests
for i in {1..10}; do
  curl "http://localhost:8080/http://httpbin.org/delay/1" &
done
wait

# Test cache performance with repeated requests
for i in {1..5}; do
  curl "http://localhost:8080/http://example.com"
done
```

### View Logs

Monitor server activity and cache performance:

```bash
# View logs for configurable server
docker compose logs -f proxy-server

# View logs for legacy configurations
docker compose --profile legacy logs -f proxy-threadpool-lru
docker compose --profile legacy logs -f proxy-high-perf
```

## Runtime Configuration (Recommended)

The new configurable approach allows you to specify server parameters at runtime using environment variables:

### Quick Start with Environment Variables

```bash
# Basic usage with default settings (Threadpool, LRUCache, port 8080, 25 threads, cache size 100)
docker compose up proxy-server

# Custom configuration using environment variables
SERVER_TYPE=Semaphore CACHE_TYPE=LFUCache PROXY_PORT=8082 THREAD_COUNT=30 CACHE_SIZE=150 docker compose up proxy-server

# High performance configuration
SERVER_TYPE=Threadpool CACHE_TYPE=LRUCache PROXY_PORT=8080 THREAD_COUNT=50 CACHE_SIZE=500 docker compose up proxy-server
```

### Using .env File

1. Copy the example environment file:
```bash
cp docker.env.example .env
```

2. Edit `.env` with your preferred settings:
```bash
SERVER_TYPE=Threadpool
CACHE_TYPE=LRUCache
PROXY_PORT=8080
THREAD_COUNT=25
CACHE_SIZE=100
CONTAINER_NAME=my-proxy-server
```

3. Start the server:
```bash
docker compose up proxy-server
```

### Configuration Options

| Variable | Options | Default | Description |
|----------|---------|---------|-------------|
| SERVER_TYPE | Threadpool, Semaphore | Threadpool | Server concurrency model |
| CACHE_TYPE | LRUCache, LFUCache | LRUCache | Cache eviction strategy |
| PROXY_PORT | Any valid port | 8080 | Port to run the server on |
| THREAD_COUNT | Positive integer | 25 | Number of threads/semaphores |
| CACHE_SIZE | Positive integer | 100 | Maximum cache entries |
| CONTAINER_NAME | String | proxy-server | Docker container name |

### Examples

```bash
# Development setup
SERVER_TYPE=Threadpool CACHE_TYPE=LRUCache PROXY_PORT=3000 THREAD_COUNT=10 CACHE_SIZE=50 docker compose up proxy-server

# Production setup
SERVER_TYPE=Threadpool CACHE_TYPE=LFUCache PROXY_PORT=80 THREAD_COUNT=100 CACHE_SIZE=1000 docker compose up proxy-server

# Testing semaphore-based server
SERVER_TYPE=Semaphore CACHE_TYPE=LRUCache PROXY_PORT=8081 THREAD_COUNT=20 CACHE_SIZE=200 docker compose up proxy-server
```

## Legacy Server Configurations

The following preset configurations are still available using profiles for backward compatibility:

| Service | Server Type | Cache Type | Port | Threads | Cache Size | Profile |
|---------|-------------|------------|------|---------|------------|---------|
| proxy-threadpool-lru | ThreadPool | LRU | 8080 | 25 | 100 | legacy |
| proxy-threadpool-lfu | ThreadPool | LFU | 8081 | 25 | 100 | legacy, threadpool-lfu |
| proxy-semaphore-lru | Semaphore | LRU | 8082 | 30 | 150 | legacy, semaphore-lru |
| proxy-semaphore-lfu | Semaphore | LFU | 8083 | 30 | 150 | legacy, semaphore-lfu |
| proxy-high-perf | ThreadPool | LRU | 8090 | 50 | 500 | legacy, high-perf |

### Using Legacy Configurations

```bash
# Run all legacy services
docker compose --profile legacy up

# Run specific legacy configuration
docker compose --profile threadpool-lfu up proxy-threadpool-lfu
```
