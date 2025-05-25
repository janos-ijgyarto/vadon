#ifndef VADON_PRIVATE_CORE_TASK_TASKSYSTEM_HPP
#define VADON_PRIVATE_CORE_TASK_TASKSYSTEM_HPP
#include <Vadon/Core/Task/TaskSystem.hpp>

#include <Vadon/Utilities/Container/Concurrent/ConcurrentQueue.hpp>

namespace Vadon::Private::Core
{
	class TaskSystem final : public Vadon::Core::TaskSystem
	{
	public:
		void enqueue_task(const Vadon::Core::Task& task) override;

		void consume_task() override;

		bool start(size_t thread_count) override;
		void request_stop() override;
		bool stop_requested() const override;
	private:
		struct TaskThread
		{
			Vadon::Utilities::ConcurrentQueue<Vadon::Core::TaskPointer> task_queue;
			std::jthread thread;

			TaskThread(TaskSystem& task_system);
			void run(TaskSystem& task_system);
		};

		TaskSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		void shutdown();

		std::vector<TaskThread> m_thread_pool;
		std::atomic<size_t> m_next_queue_index; // Round-robin feeding tasks to queues

		std::stop_source m_stop_source;

		friend class EngineCore;
	};
}
#endif