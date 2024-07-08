# Makefile for HTTP Proxy Server

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

# Linker flags
LDFLAGS = -lpthread

# Source files
SOURCES = main.cpp \
 ServerFactory.cpp \
 ThreadPoolServer.cpp \
 SemaphoreServer.cpp \
 LRUCache.cpp \
 LFUCache.cpp \
 ThreadPool.cpp \
 ProxyUtils.cpp \
 proxy_parse.c

# Object files
OBJECTS = $(SOURCES:.cpp=.o) proxy_parse.o

# Executable name
EXECUTABLE = proxy_server

# Default target
all: $(EXECUTABLE)

# Rule to create the executable
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

# Phony targets
.PHONY: all clean

# Dependencies
main.o: main.cpp ServerFactory.hpp HTTPServer.hpp
LRUCache.o: LRUCache.cpp LRUCache.hpp CacheStrategy.hpp
LFUCache.o: LFUCache.cpp LFUCache.hpp CacheStrategy.hpp
ServerFactory.o: ServerFactory.cpp ServerFactory.hpp ThreadPoolServer.hpp SemaphoreServer.hpp LRUCache.hpp LFUCache.hpp
ThreadPoolServer.o: ThreadPoolServer.cpp ThreadPoolServer.hpp HTTPServer.hpp ThreadPool.hpp ProxyUtils.hpp proxy_parse.h
SemaphoreServer.o: SemaphoreServer.cpp SemaphoreServer.hpp HTTPServer.hpp ProxyUtils.hpp proxy_parse.h
ThreadPool.o: ThreadPool.cpp ThreadPool.hpp
ProxyUtils.o: ProxyUtils.cpp ProxyUtils.hpp proxy_parse.h
proxy_parse.o: proxy_parse.c proxy_parse.h