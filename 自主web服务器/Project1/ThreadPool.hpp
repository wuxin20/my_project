#pragma once
#include <iostream>
#include <queue>
#include <pthread.h>


using namespace std;

typedef void*(*handler_t)(void*);


class Task{
private:
	int *sock_p;
	handler_t handler;
public:
	Task() :sock_p(nullptr), handler(nullptr)
	{

	}

	Task(int*sock_p_, handler_t h_) :sock_p(sock_p_), handler(h_)
	{

	}

	void Run()
	{
		handler(sock_p);
	}

	~Task()
	{

	}
};


class ThreadPool{
private:
	int num;
	queue<Task> q;
	pthread_mutex_t lock;
	pthread_cond_t cond;
public:
	ThreadPool(int num_ = 10) :num(num_)
	{
		pthread_mutex_init(&lock, nullptr);
		pthread_cond_init(&cond, nullptr);
	}

	void LockQueue()
	{
		pthread_mutex_lock(&lock);
	}

	void UnlockQueue()
	{
		pthread_mutex_unlock(&lock);
	}

	bool HasTask()
	{
		return q.size() == 0 ? false : true;
	}

	void ThreadWait()
	{
		pthread_cond_wait(&cond, &lock);
	}

	Task PopTask()
	{
		Task t = q.front();
		q.pop();
		return t;
	}

	void ThreadWakeUp()
	{
		pthread_cond_signal(&cond);
	}

	static void *ThreadRoutine(void *arg)
	{
		ThreadPool *tp = (ThreadPool*)arg;
		while (true){
			if (!tp->HasTask()){
				tp->ThreadWait();
			}
			Task t = tp->PopTask();
			tp->UnlockQueue();
			t.Run();
		}
	}

	void InitThreadPool()
	{
		pthread_t id;
		for (auto i = 0; i<num; i++){
			pthread_create(&id, nullptr, ThreadRoutine, this);
		}
	}

	void PushTask(const Task &t)
	{
		LockQueue();
		q.push(t);
		UnlockQueue();
		ThreadWakeUp();
	}

	~ThreadPool()
	{
		pthread_mutex_destroy(&lock);
		pthread_cond_destroy(&cond);
	}
};


