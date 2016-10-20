#include "threadpool.h"

ThreadPool::ThreadPool(vector<string>& queue, bool (*fn)(const string&)) : m_queue(&queue), work_fn(fn)
{
    ItemsTotal = queue.size();
    const size_t coreCount = thread::hardware_concurrency();
    NumThreads = min(ItemsTotal, coreCount);
    cout << "Creating " << NumThreads << " threads\n";
    tid.resize(NumThreads);
    for (size_t i = 0; i < NumThreads; i++) {
        pthread_create(&tid[i], nullptr, ThreadFunction, this);
        tid.push_back(i);
    }
}

ThreadPool::~ThreadPool()
{
    for (size_t i = 0; i < NumThreads; i++) {
        pthread_join(tid[i], nullptr);
    }
    cout << "Successfully processed: " << ItemsCompleted << "/" << ItemsTotal << " items" << '\n';
}

void* ThreadPool::ThreadFunction(void *arg)
{
    auto *ctx = (ThreadPool*)arg;

    while (1) {
        pthread_mutex_lock(&ctx->queue_mutex);
        if (ctx->m_queue->empty()) {
            pthread_mutex_unlock(&ctx->queue_mutex);
            break;
        }
        string cur (ctx->m_queue->back());
        ctx->m_queue->pop_back();
        pthread_mutex_unlock(&ctx->queue_mutex);
        if (ctx->work_fn(cur)) // Increase counter on success
            ctx->ItemsCompleted++;
    }
    return nullptr;
}
