#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Core/Task/TaskSystem.hpp>

#include <concurrent_queue.h>

#include <format>

namespace Vadon::Private::Core
{	
	// TODO: implement a "thread context" setup (via thread local static) so task threads can work on a dedicated object
	// For example, task nodes/groups could be allocated by each thread on their own dedicated pool
	TaskSystem::TaskThread::TaskThread(TaskSystem& task_system)
		: thread([this](TaskSystem& task_system) { run(task_system); }, std::ref(task_system))
	{
	}

	void TaskSystem::TaskThread::run(TaskSystem& task_system)
	{
		std::shared_ptr<Vadon::Core::Task> pending_task;
		std::stop_token stop_token = task_system.m_stop_source.get_token();

		while (!stop_token.stop_requested())
		{
			// TODO: steal tasks from other threads?
			if (task_queue.try_dequeue(pending_task))
			{
				(*pending_task)();
				pending_task.reset();
			}

			// Wait until we get more work
			std::this_thread::yield();
		}
	}

	void TaskSystem::enqueue_task(const Vadon::Core::Task& task)
	{
		if (m_stop_source.stop_requested())
		{
			// Do not attempt to queue tasks once we requested a stop
			// FIXME: profile this, might be a performance concern
			return;
		}

		// Get the queue index and update the counter
		const size_t queue_index = m_next_queue_index.fetch_add(1, std::memory_order_relaxed) % m_thread_pool.size();
		
		TaskThread& selected_thread = m_thread_pool[queue_index];
		selected_thread.task_queue.enqueue(std::make_shared<Vadon::Core::Task>(task));
	}

	void TaskSystem::consume_task()
	{
		if (m_thread_pool.empty())
		{
			// No task threads have been added
			log_error("Task system has no running task threads!\n");
			return;
		}

		// Attempt to "steal" a task from one of the workers
		std::shared_ptr<Vadon::Core::Task> pending_task;
		for(TaskThread& current_task_thread : m_thread_pool)
		{
			if (current_task_thread.task_queue.try_dequeue(pending_task))
			{
				(*pending_task)();
				break;
			}
		}
	}

	bool TaskSystem::start(const Vadon::Core::TaskConfiguration& task_config)
	{
		if (!m_thread_pool.empty() && !stop_requested())
		{
			log_error("Task system is already running!\n");
			return false;
		}

		log_message("Initializing Task System\n");

		if (task_config.thread_count <= 0)
		{
			log_error("Invalid task thread count!\n");
			return true;
		}

		log_message(std::format("Creating {} task threads\n", task_config.thread_count));

		// Reset stop source
		m_stop_source = std::stop_source();

		// Create the threads
		m_thread_pool.clear();
		m_thread_pool.reserve(task_config.thread_count);
		while (static_cast<int32_t>(m_thread_pool.size()) < task_config.thread_count)
		{
			m_thread_pool.emplace_back(*this);
		}

		log_message("Task System initialized successfully!\n");
		return true;
	}

	void TaskSystem::request_stop()
	{
		m_stop_source.request_stop();
	}

	bool TaskSystem::stop_requested() const
	{
		return m_stop_source.stop_requested();
	}

	TaskSystem::TaskSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Core::TaskSystem(core)
		, m_next_queue_index(0)
	{

	}

	bool TaskSystem::initialize()
	{
		// TODO: anything?
		return true;
	}

	void TaskSystem::shutdown()
	{
		log_message("Shutting down Task System\n");

		// Make sure we stopped, clear the thread pool
		request_stop();
		m_thread_pool.clear();

		log_message("Task System shut down successfully!\n");
	}
}