#ifndef VADON_CORE_TASK_TASK_HPP
#define VADON_CORE_TASK_TASK_HPP
#include <Vadon/Common.hpp>
#include <memory>
#include <functional>
#include <mutex>
namespace Vadon::Core
{
	using Task = std::function<void()>;
	using TaskPointer = std::shared_ptr<Task>; // FIXME: use frame allocator instead of shared_ptr!
	using TaskVector = std::vector<Task>;

	class TaskNodeData;
	using TaskNode = std::shared_ptr<TaskNodeData>; // FIXME: use frame allocator instead of shared_ptr!
	using TaskNodeVector = std::vector<TaskNode>;

	class TaskGroupData;
	using TaskGroup = std::shared_ptr<TaskGroupData>; // FIXME: use frame allocator instead of shared_ptr!

	class TaskSystem;

	// TODO: metadata to help with debugging?
	// e.g named nodes/tasks/etc.

	// Simple task graph node: contains a dependency counter, dependent nodes, and subtasks
	// - If dependency counter is zero on update, it will first run all subtasks, then decrement the counters of dependent nodes and update
	// NOTE: adding dependents and subtasks is NOT thread-safe, and attempting to update after initial run is UB
	class TaskNodeData
	{
	public:
		void VADONCOMMON_API add_dependent(const TaskNode& node);
		TaskNode VADONCOMMON_API create_dependent(std::string_view name);

		void VADONCOMMON_API add_subtask(const Task& task);

		void VADONCOMMON_API update();
	private:
		TaskNodeData(std::string_view name, TaskSystem& task_system);

		void dependency_completed();

		const std::string m_name;
		TaskSystem& m_task_system;

		std::atomic<int32_t> m_dependency_counter;
		std::once_flag m_once_flag; // Ensures that tasks are only run once

		TaskNodeVector m_dependent_nodes;
		TaskVector m_subtasks;

		friend TaskSystem;
	};

	// Special task node that will become a "sync point": task nodes can be generated and added to the group, ensuring that all are run before the group is done
	// NOTE: 
	// - Start nodes are expected to be added before group is run. Adding afterward is UB!
	// - End dependencies can be added dynamically. Adding after end node runs is UB!
	class TaskGroupData
	{
	public:
		void add_start_subtask(const Task& task) { m_start_node->add_subtask(task); }
		
		void add_start_dependent(const TaskNode& node) { m_start_node->add_dependent(node); add_end_dependency(node); }
		void add_start_dependent(const TaskGroup& group) { add_start_dependent(group->m_start_node); add_end_dependency(group); }

		TaskNode create_start_dependent(std::string_view name) { TaskNode node = m_start_node->create_dependent(name); add_end_dependency(node); return node; }

		void add_start_dependency(const TaskNode& node) { node->add_dependent(m_start_node); }		

		void add_end_dependency(const TaskNode& node) { node->add_dependent(m_end_node); }
		void add_end_dependency(const TaskGroup& group) { add_end_dependency(group->m_end_node); }

		void add_end_subtask(const Task& task) { m_end_node->add_subtask(task); }

		void add_end_dependent(const TaskNode& node) { m_end_node->add_dependent(node); }
		void add_end_dependent(const TaskGroup& group) { add_end_dependent(group->m_start_node); }

		TaskNode create_end_dependent(std::string_view name) { return m_end_node->create_dependent(name); }

		void update() { m_start_node->update(); }
	private:
		TaskGroupData(std::string_view name, TaskSystem& task_system);

		const std::string m_name;
		TaskSystem& m_task_system;

		TaskNode m_start_node; // Run before all internal nodes are run
		TaskNode m_end_node; // Run once all start and internal dependencies are completed

		friend TaskSystem;
	};
}
#endif