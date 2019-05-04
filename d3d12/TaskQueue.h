#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <set>

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

	size_t ThreadCount() { return workers_.size(); }

	void Setup(int threadCount)
	{
		auto& queue = taskQueue_;
		auto& onGoingTaskPtrs = onGoingTaskPtrs_;
		auto& queueLock = queueLock_;
		auto& enqueueEvent = enqueueEvent_;
		auto& emptyEvent = emptyEvent_;
		auto& isExited = isExited_;

		for (auto i = 0; i < threadCount; ++i)
		{
			workers_.emplace_back([&queue, &onGoingTaskPtrs, &queueLock, &enqueueEvent, &emptyEvent, &isExited]()
			{
				while (true)
				{
					std::function<void()> task;
					bool doTask = false;

					{
						std::unique_lock<std::mutex> lk(queueLock);
						enqueueEvent.wait(lk, [&queue, &isExited]() 
						{
							return isExited || !queue.empty();
						});

						if (isExited)
						{
							break;
						}

						doTask = !queue.empty();

						if (doTask)
						{
							task = std::move(queue.front());
							queue.pop();
							onGoingTaskPtrs.insert(&task);
						}
					}

					if (doTask)
					{
						task();
					}

					{
						std::unique_lock<std::mutex> lk(queueLock);
						onGoingTaskPtrs.erase(&task);
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

		enqueueEvent_.notify_all();
		while (!(queue.empty() && onGoingTaskPtrs_.empty()))
		{
			std::unique_lock<std::mutex> lk(emptyLock_);
			emptyEvent_.wait(lk, [&queue]() { return queue.empty(); });
		}
	}

private:
	std::vector<std::thread> workers_;
	std::queue<std::function<void()>> taskQueue_;
	std::set<std::function<void()>*> onGoingTaskPtrs_;

	std::mutex queueLock_;
	std::condition_variable enqueueEvent_;

	std::mutex emptyLock_;
	std::condition_variable emptyEvent_;

	bool isExited_ = false;
};

