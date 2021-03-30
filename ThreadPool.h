#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <future>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <stdexcept>

struct ThreadPool{

public:
	ThreadPool(size_t);

	template<class F, class... Args>
	auto equeue(F&& f, Args&& ... args)
		->std::future<typename std::result_of<F(Args...)>::type >;

	~ThreadPool();

private:
	int threads;
	std::vector<std::thread> workers;
	std::queue<std::function<void()> > tasks;

	std::mutex queue_mutex;
	std::condition_variable condition;

	bool stop;
};


inline ThreadPool::ThreadPool(size_t threads):stop(false)
{
	for (int i = 0; i < threads; i++) {
		workers.emplace_back(
			[this]{
				for (;;) {					
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						condition.wait(lock, [this] {return this->stop || !this->tasks.empty(); });

						if (this->stop && this->tasks.empty())
							return;

						task = std::move(tasks.front());
						this->tasks.pop();
					}
					task();
				}
			}
		);
	}

}

inline ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(this->queue_mutex);
		this->stop = true;
	}

	this->condition.notify_all();

	for (std::thread &work : this->workers) {
		work.join();
	}

}

template<class F, class ...Args>
auto ThreadPool::equeue(F&& f, Args&& ...args) -> std::future<typename std::result_of<F(Args ...)>::type>
{
	using return_type = typename std::result_of<F(Args ...)>::type;
	auto task = std::make_shared<std::packaged_task<return_type()>>(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...));

	std::future<return_type> res = task->get_future();

	{
		std::unique_lock<std::mutex> lock(this->queue_mutex);

		if (stop)
			throw std::runtime_error("equeue on stopped ThreadPool");

		tasks.emplace(
			[task]() {(*task)(); }
		);
	}
	condition.notify_one();

	return res;
}