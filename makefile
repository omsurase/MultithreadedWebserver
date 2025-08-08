# Makefile for HTTP Proxy Server

## Makefile for HTTP Proxy Server (reorganized src/include layout)

# Compilers
CXX = g++
CC = gcc

# Directories
SRC_DIR = src
INC_DIR = include

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -I$(INC_DIR) -I$(INC_DIR)/c
CFLAGS = -Wall -Wextra -pedantic -I$(INC_DIR) -I$(INC_DIR)/c

# Linker flags
LDFLAGS = -lpthread

# Source files
SOURCES_CPP = \
 $(SRC_DIR)/main.cpp \
 $(SRC_DIR)/core/ThreadPool.cpp \
 $(SRC_DIR)/servers/ThreadPoolServer.cpp \
 $(SRC_DIR)/servers/SemaphoreServer.cpp \
 $(SRC_DIR)/cache/LRUCache.cpp \
 $(SRC_DIR)/cache/LFUCache.cpp \
 $(SRC_DIR)/utils/ProxyUtils.cpp \
 $(SRC_DIR)/core/ServerFactory.cpp

SOURCES_C = \
 $(SRC_DIR)/c/proxy_parse.c

# Object files
OBJECTS = $(SOURCES_CPP:.cpp=.o) $(SOURCES_C:.c=.o)

# Executable name
EXECUTABLE = proxy_server

# Default target
all: $(EXECUTABLE)

# Rule to create the executable
$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Compile C++ sources
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C sources
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

# Phony targets
.PHONY: all clean

# Convenience dependencies (non-exhaustive)
$(SRC_DIR)/core/ThreadPool.o: $(INC_DIR)/core/ThreadPool.hpp
$(SRC_DIR)/servers/ThreadPoolServer.o: $(INC_DIR)/servers/ThreadPoolServer.hpp $(INC_DIR)/utils/ProxyUtils.hpp $(INC_DIR)/c/proxy_parse.h
$(SRC_DIR)/servers/SemaphoreServer.o: $(INC_DIR)/servers/SemaphoreServer.hpp $(INC_DIR)/utils/ProxyUtils.hpp $(INC_DIR)/c/proxy_parse.h
$(SRC_DIR)/utils/ProxyUtils.o: $(INC_DIR)/utils/ProxyUtils.hpp $(INC_DIR)/c/proxy_parse.h
$(SRC_DIR)/cache/LRUCache.o: $(INC_DIR)/cache/LRUCache.hpp $(INC_DIR)/cache/CacheStrategy.hpp
$(SRC_DIR)/cache/LFUCache.o: $(INC_DIR)/cache/LFUCache.hpp $(INC_DIR)/cache/CacheStrategy.hpp
$(SRC_DIR)/core/ServerFactory.o: $(INC_DIR)/core/ServerFactory.hpp $(INC_DIR)/servers/ThreadPoolServer.hpp $(INC_DIR)/servers/SemaphoreServer.hpp $(INC_DIR)/cache/LRUCache.hpp $(INC_DIR)/cache/LFUCache.hpp