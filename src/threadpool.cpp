#include "threadpool.h"

using std::cout;
using std::vector;
using std::exception;
using std::string;
using std::thread;

ThreadPool::ThreadPool(vector<string>& queue, bool (*fn)(const string&)) : queue(&queue), work_fn(fn)
{
    ItemsTotal = queue.size();
    const size_t coreCount = thread::hardware_concurrency();
    NumThreads = std::min(ItemsTotal, coreCount);
    cout << "Creating " << NumThreads << " threads\n";
    tid.resize(NumThreads);
    for (auto &thread: tid) {
        auto res = pthread_create(&thread, nullptr, ThreadFunction, this);
        switch (res) {
        case 0:
            break;
        case EAGAIN:
            throw std::runtime_error("Insufficient resources to create thread.");
            break;
        case EINVAL:
            throw std::runtime_error("Invalid settings in attr.");
            break;
        case EPERM:
            throw std::runtime_error("Insufficitent permissions.");
            break;
        default:
            throw std::runtime_error("Unknown error creating thread.");
        }
    }
}

ThreadPool::~ThreadPool()
{
    for (auto thread: tid) {
        pthread_join(thread, nullptr);
    }
    cout << "Successfully processed: " << ItemsCompleted << "/" << ItemsTotal << " items" << '\n';
}

void* ThreadPool::ThreadFunction(void *arg)
{
    auto *ctx = (ThreadPool*)arg;

    while (1) {
        pthread_mutex_lock(&ctx->queue_mutex);
        if (ctx->queue->empty()) {
            pthread_mutex_unlock(&ctx->queue_mutex);
            break;
        }
        string cur(ctx->queue->back());
        ctx->queue->pop_back();
        pthread_mutex_unlock(&ctx->queue_mutex);
        if (ctx->work_fn(cur)) // Increase counter on success
            ctx->ItemsCompleted++;
    }
    return nullptr;
}
