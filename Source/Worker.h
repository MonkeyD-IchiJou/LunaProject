#ifndef WORKERTHRD_H
#define WORKERTHRD_H

#include <thread>
#include <mutex>
#include <atomic>
#include <queue>

namespace luna
{
	class Worker
	{
	public:
		Worker();
		~Worker();
		
		// add new job to the thrd's queue
		void addJob(std::function<void()> NewJob);

		// wait until all the job finish execute
		void wait();

	private:
		void Infinite_loop_function();

	private:
		std::queue<std::function<void()>> m_jobqueues{};
		std::thread m_workerthrd;
		std::mutex m_queuemutex{};
		std::condition_variable m_condition{};
		std::atomic<bool> m_loop = true;
	};
}

#endif

