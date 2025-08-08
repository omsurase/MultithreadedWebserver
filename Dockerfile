FROM debian:bookworm-slim AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       build-essential \
       make \
       ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source
COPY . /app

# Reorganize sources inside the container if still flat
RUN set -eux; \
    mkdir -p src/core src/servers src/cache src/utils src/c include/core include/servers include/cache include/utils include/c examples || true; \
    [ -f main.cpp ] && mv -f main.cpp src/main.cpp || true; \
    [ -f ThreadPool.cpp ] && mv -f ThreadPool.cpp src/core/ThreadPool.cpp || true; \
    [ -f ServerFactory.cpp ] && mv -f ServerFactory.cpp src/core/ServerFactory.cpp || true; \
    [ -f ThreadPoolServer.cpp ] && mv -f ThreadPoolServer.cpp src/servers/ThreadPoolServer.cpp || true; \
    [ -f SemaphoreServer.cpp ] && mv -f SemaphoreServer.cpp src/servers/SemaphoreServer.cpp || true; \
    [ -f LRUCache.cpp ] && mv -f LRUCache.cpp src/cache/LRUCache.cpp || true; \
    [ -f LFUCache.cpp ] && mv -f LFUCache.cpp src/cache/LFUCache.cpp || true; \
    [ -f ProxyUtils.cpp ] && mv -f ProxyUtils.cpp src/utils/ProxyUtils.cpp || true; \
    [ -f proxy_parse.c ] && mv -f proxy_parse.c src/c/proxy_parse.c || true; \
    [ -f HTTPServer.hpp ] && mv -f HTTPServer.hpp include/core/HTTPServer.hpp || true; \
    [ -f ThreadPool.hpp ] && mv -f ThreadPool.hpp include/core/ThreadPool.hpp || true; \
    [ -f ServerFactory.hpp ] && mv -f ServerFactory.hpp include/core/ServerFactory.hpp || true; \
    [ -f ThreadPoolServer.hpp ] && mv -f ThreadPoolServer.hpp include/servers/ThreadPoolServer.hpp || true; \
    [ -f SemaphoreServer.hpp ] && mv -f SemaphoreServer.hpp include/servers/SemaphoreServer.hpp || true; \
    [ -f CacheStrategy.hpp ] && mv -f CacheStrategy.hpp include/cache/CacheStrategy.hpp || true; \
    [ -f LRUCache.hpp ] && mv -f LRUCache.hpp include/cache/LRUCache.hpp || true; \
    [ -f LFUCache.hpp ] && mv -f LFUCache.hpp include/cache/LFUCache.hpp || true; \
    [ -f ProxyUtils.hpp ] && mv -f ProxyUtils.hpp include/utils/ProxyUtils.hpp || true; \
    [ -f proxy_parse.h ] && mv -f proxy_parse.h include/c/proxy_parse.h || true; \
    [ -d v1 ] && mv -f v1 examples/v1 || true

# Build
RUN make clean && make -j

# Runtime image
FROM debian:bookworm-slim

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       ca-certificates \
       libstdc++6 \
       libgcc1 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=build /app/proxy_server /app/proxy_server

EXPOSE 8080

# Default: Threadpool server, LRU cache, port 8080, 25 threads, cache size 100
ENTRYPOINT ["/app/proxy_server"]
CMD ["Threadpool", "LRUCache", "8080", "25", "100"]

