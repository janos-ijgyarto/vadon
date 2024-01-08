#ifndef VADON_CORE_TASK_TASKSYSTEM_HPP
#define VADON_CORE_TASK_TASKSYSTEM_HPP
#include <Vadon/Core/CoreModule.hpp>
#include <Vadon/Core/Task/Task.hpp>
namespace Vadon::Core
{
	class TaskSystem : public CoreSystem<TaskSystem>
	{
	public:
		VADONCOMMON_API TaskNode create_task_node(std::string_view name);
		VADONCOMMON_API TaskGroup create_task_group(std::string_view name);

		virtual void enqueue_task(const Task& task) = 0;

		// Allows a thread to enter and attempt to "steal" an enqueued task from the system's threads
		// Mainly useful for giving work to the main thread
		virtual void consume_task() = 0;

		virtual bool start(const TaskConfiguration& task_config) = 0;
		virtual void request_stop() = 0;
		virtual bool stop_requested() const = 0;
	protected:
		TaskSystem(EngineCoreInterface& core) 
			: System(core) 
		{
		}
	};
}
#endif