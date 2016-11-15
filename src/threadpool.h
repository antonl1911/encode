#pragma once
#include <thread>
#include <vector>
#include <iostream>
#include <pthread.h>


class ThreadPool {
    std::vector<pthread_t> tid;
    std::vector<std::string> *queue;
    static void* ThreadFunction(void *arg);
    bool (*work_fn)(const std::string&);
    size_t ItemsTotal = 0;
    size_t ItemsCompleted = 0;
    size_t NumThreads = 0;
    pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
public:
    ThreadPool(std::vector<std::string>& queue, bool (*fn)(const std::string&));
    ThreadPool() = delete;
    ~ThreadPool();
};
