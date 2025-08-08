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
# View logs for default server
docker compose logs -f proxy-threadpool-lru

# View logs for specific configurations
docker compose logs -f proxy-threadpool-lfu
docker compose logs -f proxy-semaphore-lru
docker compose logs -f proxy-high-perf
```

## Server Configurations

| Service | Server Type | Cache Type | Port | Threads | Cache Size |
|---------|-------------|------------|------|---------|------------|
| proxy-threadpool-lru | ThreadPool | LRU | 8080 | 25 | 100 |
| proxy-threadpool-lfu | ThreadPool | LFU | 8081 | 25 | 100 |
| proxy-semaphore-lru | Semaphore | LRU | 8082 | 30 | 150 |
| proxy-semaphore-lfu | Semaphore | LFU | 8083 | 30 | 150 |
| proxy-high-perf | ThreadPool | LRU | 8090 | 50 | 500 |
