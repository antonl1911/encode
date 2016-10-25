#include "threadpool.h"

ThreadPool::ThreadPool(vector<string>& queue, bool (*fn)(const string&)) : m_queue(&queue), work_fn(fn)
{
    ItemsTotal = queue.size();
    const size_t coreCount = thread::hardware_concurrency();
    NumThreads = std::min(ItemsTotal, coreCount);

    cout << "Creating " << NumThreads << " threads\n";
    tid.resize(NumThreads);
    for (size_t i = 0; i < NumThreads; i++) {
        tid[i] = std::thread(ThreadFunction, std::ref(*this));
    }

}

ThreadPool::~ThreadPool()
{
    for (size_t i = 0; i < NumThreads; i++) {
        tid[i].join();
    }
    cout << "Successfully processed: " << ItemsCompleted << "/" << ItemsTotal << " items" << '\n';
}

void ThreadPool::ThreadFunction(ThreadPool& ctx)
{
    while (1) {
        ctx.queue_mutex.lock(); 
        if (ctx.m_queue->empty()) {
            ctx.queue_mutex.unlock(); 
            break;
        }
        string cur(ctx.m_queue->back());
        ctx.m_queue->pop_back();
        ctx.queue_mutex.unlock();
        if (ctx.work_fn(cur)) // Increase counter on success
            ctx.ItemsCompleted++;
    }
}
