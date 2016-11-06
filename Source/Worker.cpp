#include "Worker.h"
#include "DebugLog.h"

namespace luna
{
	Worker::Worker() :  m_loop(true)
	{
		// start the thread
		m_workerthrd = std::thread(&Worker::Infinite_loop_function, this);
	}

	Worker::~Worker()
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

	void Worker::addJob(std::function<void()> NewJob)
	{
		std::lock_guard<std::mutex> lock(m_queuemutex);
		m_jobqueues.push(NewJob);

		// inform the loop about this new job
		m_condition.notify_one();
	}

	void Worker::wait()
	{
		std::unique_lock<std::mutex> lock(m_queuemutex);
		m_condition.wait(lock, [this]() { 
			return m_jobqueues.empty(); 
		});
	}

	void Worker::Infinite_loop_function()
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

#if _DEBUG
			try
			{
				job();
			}
			catch (const std::runtime_error& e)
			{
				DebugLog::printF("\n [Worker Thread Catch]->");
				DebugLog::printF(e.what());
				system("pause");
			}
#else
			job();
#endif

			{
				std::lock_guard<std::mutex> lock(m_queuemutex);
				m_jobqueues.pop();
				m_condition.notify_one();
			}
		}
	}
}
