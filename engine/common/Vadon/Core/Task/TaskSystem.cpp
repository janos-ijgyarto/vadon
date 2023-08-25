#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Core/Task/TaskSystem.hpp>

namespace Vadon::Core
{
	TaskNode TaskSystem::create_task_node(std::string_view name)
	{		
		return std::shared_ptr<TaskNodeData>(new TaskNodeData(name, *this));
	}

	TaskGroup TaskSystem::create_task_group(std::string_view name)
	{
		return std::shared_ptr<TaskGroupData>(new TaskGroupData(name, *this));
	}
}