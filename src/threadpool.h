#pragma once
#include <thread>
#include <vector>
#include <iostream>
#include <pthread.h>

using namespace std;

class ThreadPool {
    vector<pthread_t> tid;
    vector<string> *m_queue;
    static void* ThreadFunction(void *arg);
    bool (*work_fn)(const string&);
    size_t ItemsTotal = 0;
    size_t ItemsCompleted = 0;
    size_t NumThreads = 0;
    pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
public:
    ThreadPool(vector<string>& queue, bool (*fn)(const string&));
    ThreadPool() = delete;
    ~ThreadPool();
};
