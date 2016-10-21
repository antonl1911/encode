#pragma once
#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <functional>

using namespace std;

class ThreadPool {
    vector<thread> tid;
    vector<string> *m_queue;
    static void ThreadFunction(ThreadPool& ctx);
    bool (*work_fn)(const string&);
    size_t ItemsTotal = 0;
    size_t ItemsCompleted = 0;
    size_t NumThreads = 0;
    mutex queue_mutex;
public:
    ThreadPool(vector<string>& queue, bool (*fn)(const string&));
    ThreadPool() = delete;
    ~ThreadPool();
};
