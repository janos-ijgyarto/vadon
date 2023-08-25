#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Core/Task/Task.hpp>

#include <Vadon/Core/Task/TaskSystem.hpp>

namespace Vadon::Core
{
	void TaskNodeData::add_dependent(const TaskNode& node)
	{
		// Add node to dependent list, and increment its dependency counter
		m_dependent_nodes.push_back(node);

		++node->m_dependency_counter;
	}

	TaskNode TaskNodeData::create_dependent(std::string_view name)
	{
		TaskNode node = m_task_system.create_task_node(name);
		add_dependent(node);

		return node;
	}

	void TaskNodeData::add_subtask(const Task& task)
	{
		m_subtasks.push_back(task);
	}

	void TaskNodeData::update()
	{
		const int32_t dependency_count = m_dependency_counter.load();
		if (dependency_count > 0)
		{
			// Task still has dependencies that need to be completed first
			return;
		}

		std::call_once(m_once_flag,
			[this]()
			{
				// All dependencies are completed, perform work attached to node
				if (m_subtasks.empty())
				{
					// No subtasks, update dependents directly
					for (const TaskNode& current_dependent : m_dependent_nodes)
					{
						current_dependent->dependency_completed();
					}

					return;
				}

				if (m_dependent_nodes.empty())
				{
					// No dependents, just queue all subtasks
					for (const Task& current_subtask : m_subtasks)
					{
						m_task_system.enqueue_task(current_subtask);
					}

					return;
				}

				// Cache the dependent nodes in a temporary shared object (the node itself may end up being dereferenced)
				struct TaskNodeTempData
				{
					TaskNodeVector dependent_nodes;

					~TaskNodeTempData()
					{
						// On destruction, we update the dependents
						for (const TaskNode& current_dependent : dependent_nodes)
						{
							current_dependent->dependency_completed();
						}
					}
				};

				std::shared_ptr<TaskNodeTempData> temp_data = std::make_shared<TaskNodeTempData>(m_dependent_nodes);

				// Wrap each subtask in a lambda that captures the temp data (will be cleared once all subtasks are done)
				for (const Task& current_subtask : m_subtasks)
				{
					auto subtask_wrapper = [temp_data, current_subtask]()
					{
						current_subtask();
					};

					// Queue the subtasks
					m_task_system.enqueue_task(subtask_wrapper);
				}
			}
		);		
	}

	TaskNodeData::TaskNodeData(std::string_view name, TaskSystem& task_system)
		: m_name(name)
		, m_task_system(task_system)
		, m_dependency_counter(0)
	{
	}

	void TaskNodeData::dependency_completed()
	{
		const int32_t current_dependency_count = --m_dependency_counter;
		if (current_dependency_count > 0)
		{
			return;
		}

		update();
	}

	TaskGroupData::TaskGroupData(std::string_view name, TaskSystem& task_system)
		: m_name(name)
		, m_task_system(task_system)
	{
		m_start_node = task_system.create_task_node(m_name + " - begin");
		m_end_node = task_system.create_task_node(m_name + " - end");

		// End node depends on start
		m_start_node->add_dependent(m_end_node);
	}
}