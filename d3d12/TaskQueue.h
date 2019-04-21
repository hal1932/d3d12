#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <queue>
#include <iostream>

class TaskQueue
{
public:
	~TaskQueue()
	{
		isExited_ = true;
		enqueueEvent_.notify_all();

		WaitAll();

		for (auto& th : workers_)
		{
			if (th.joinable())
			{
				th.join();
			}
		}
	}

	int ThreadCount() { return static_cast<int>(workers_.size()); }

	void Setup(int threadCount)
	{
		auto& queue = taskQueue_;
		auto& queueLock = queueLock_;
		auto& enqueueEvent = enqueueEvent_;
		auto& emptyEvent = emptyEvent_;
		auto& isExited = isExited_;

		for (auto i = 0; i < threadCount; ++i)
		{
			workers_.emplace_back([&queue, &queueLock, &enqueueEvent, &emptyEvent, &isExited]()
			{
				while (true)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lk(queueLock);
						enqueueEvent.wait(lk, [&queue, &isExited]() 
						{
							return isExited || !queue.empty();
						});

						if (isExited && queue.empty())
						{
							break;
						}

						task = std::move(queue.front());
						queue.pop();
					}

					task();

					{
						std::unique_lock<std::mutex> lk(queueLock);
						if (queue.empty())
						{
							emptyEvent.notify_all();
						}
					}
				}
			});
		}
	}

	void Enqueue(std::function<void()> task)
	{
		{
			std::unique_lock<std::mutex> lk(queueLock_);
			taskQueue_.emplace(task);
		}
		enqueueEvent_.notify_one();
	}

	void WaitAll()
	{
		auto& queue = taskQueue_;

		std::unique_lock<std::mutex> lk(emptyLock_);
		emptyEvent_.wait(lk, [&queue]() { return queue.empty(); });
	}

private:
	std::vector<std::thread> workers_;
	std::queue<std::function<void()>> taskQueue_;

	std::mutex queueLock_;
	std::condition_variable enqueueEvent_;

	std::mutex emptyLock_;
	std::condition_variable emptyEvent_;

	bool isExited_ = false;
};

