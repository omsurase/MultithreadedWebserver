// main.cpp
#include <iostream>
#include <string>
#include <memory>
#include "./ServerFactory.hpp"
#include "./HTTPServer.hpp"

int main(int argc, char *argv[])
{
    if (argc < 4 || argc > 6)
    {
        std::cerr << "Usage: " << argv[0] << " <server_type> <cache_type> <port> [num_threads] [cache_size]" << std::endl;
        std::cerr << "Server types: Threadpool, Semaphore" << std::endl;
        std::cerr << "Cache types: LRUCache, LFUCache" << std::endl;
        std::cerr << "Port: Any valid port number (e.g., 8080)" << std::endl;
        std::cerr << "Num threads: (Optional) Number of threads for the server" << std::endl;
        std::cerr << "Cache size: (Optional) Maximum number of elements in the cache" << std::endl;
        return 1;
    }

    std::string serverType = argv[1];
    std::string cacheType = argv[2];
    int port, numThreads = 25, cacheSize = 100; // Default values

    try
    {
        port = std::stoi(argv[3]);
        if (argc > 4)
            numThreads = std::stoi(argv[4]);
        if (argc > 5)
            cacheSize = std::stoi(argv[5]);
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Error: Invalid argument. Please provide valid numbers for port, num_threads, and cache_size." << std::endl;
        return 1;
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << "Error: Number out of range for port, num_threads, or cache_size." << std::endl;
        return 1;
    }

    try
    {
        std::unique_ptr<HTTPServer> server = ServerFactory::createHTTPServer(serverType, cacheType, numThreads, cacheSize);
        std::cout << "Starting HTTP proxy server on port " << port << std::endl;
        std::cout << "Number of threads: " << numThreads << std::endl;
        std::cout << "Cache size: " << cacheSize << std::endl;
        server->start(port);
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}