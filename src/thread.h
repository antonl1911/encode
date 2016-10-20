#pragma once
#include <thread>
#include <vector>
#include <pthread.h>
#include <lame/lame.h>

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


ThreadPool::ThreadPool(vector<string>& queue, bool (*fn)(const string&)) : work_fn(fn), m_queue(&queue) 
{
	ItemsTotal = queue.size();
	const size_t coreCount = thread::hardware_concurrency();
	NumThreads = min(ItemsTotal, coreCount);
	cout << "Creating " << NumThreads << " threads\n";
	tid.resize(NumThreads);
	for (int i = 0; i < NumThreads; i++)
	{
		pthread_create(&tid[i], NULL, ThreadFunction, this);
		tid.push_back(i);
	}
}

ThreadPool::~ThreadPool()
{
	for (int i = 0; i < NumThreads; i++)
	{
		pthread_join(tid[i], NULL);
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
		bool result = ctx->work_fn(cur);
		if (result)
			ctx->ItemsCompleted++;
	}
	pthread_exit(nullptr);
}

