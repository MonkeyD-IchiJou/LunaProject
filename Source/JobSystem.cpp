#include "JobSystem.h"

namespace luna
{
	JobSystem::JobSystem()
	{
		// start the thread
		m_workerthrd = std::thread(&JobSystem::Infinite_loop_function, this);
	}

	JobSystem::~JobSystem()
	{
		if (m_workerthrd.joinable())
		{
			wait();
			m_queuemutex.lock();
			m_loop = false;
			m_condition.notify_one();
			m_queuemutex.unlock();
			m_workerthrd.join();
		}
	}

	void JobSystem::addJob(std::function<void()> NewJob)
	{
		std::lock_guard<std::mutex> lock(m_queuemutex);
		m_jobqueues.push(NewJob);

		// inform the loop about this new job
		m_condition.notify_one();
	}

	void JobSystem::wait()
	{
		std::unique_lock<std::mutex> lock(m_queuemutex);
		m_condition.wait(lock, [this]() { 
			return m_jobqueues.empty(); 
		});
	}

	void JobSystem::Infinite_loop_function()
	{
		while (true)
		{
			std::function<void()> job{};

			{
				std::unique_lock<std::mutex> lock(m_queuemutex);

				// wait until the queues is not empty || m_loop is false
				m_condition.wait(lock, [this] { return !m_jobqueues.empty() || !m_loop; });

				if (m_loop == false)
				{
					break;
				}

				job = m_jobqueues.front();
			}

			job();

			{
				std::lock_guard<std::mutex> lock(m_queuemutex);
				m_jobqueues.pop();
				m_condition.notify_one();
			}
		}
	}
}
