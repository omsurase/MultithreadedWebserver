#include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t numThreads) : stop(false), readyThreads(0)
{
    for (size_t i = 0; i < numThreads; ++i)
    {
        workers.emplace_back([this]
                             {
            {
                std::unique_lock<std::mutex> lock(this->queueMutex);
                readyThreads++;
                allThreadsReady.notify_one();
            }
            for(;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    if(this->stop && this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            } });
    }
}

ThreadPool::~ThreadPool()
{
    stop = true;
    condition.notify_all();
    for (std::thread &worker : workers)
        worker.join();
}

void ThreadPool::enqueue(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.emplace(task);
    }
    condition.notify_one();
}

void ThreadPool::waitForThreads()
{
    std::unique_lock<std::mutex> lock(queueMutex);
    allThreadsReady.wait(lock, [this]
                         { return readyThreads == workers.size(); });
}