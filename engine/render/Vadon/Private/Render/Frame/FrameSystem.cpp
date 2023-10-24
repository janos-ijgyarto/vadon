#include <Vadon/Private/PCH/Render.hpp>
#include <Vadon/Private/Render/Frame/FrameSystem.hpp>

#include <Vadon/Core/CoreInterface.hpp>

namespace Vadon::Private::Render
{
	FrameGraphHandle FrameSystem::create_graph(const FrameGraphInfo& graph_info)
	{
		// Only thing we care about is whether a pass needs the output of another pass
		using PassIndexVector = std::vector<size_t>;
		std::unordered_map<std::string, PassIndexVector> resource_lookup;

		{
			size_t current_pass_index = 0;
			for (const Vadon::Render::RenderPass& current_pass : graph_info.passes)
			{
				for (const Vadon::Render::PassResource& current_target : current_pass.targets)
				{
					if (current_target.out_name.empty())
					{
						continue;
					}

					PassIndexVector& node_indices = resource_lookup[current_target.out_name];
					node_indices.push_back(current_pass_index);
				}
				++current_pass_index;
			}
		}

		std::unordered_map<size_t, PassIndexVector> pass_child_lookup;
		std::vector<bool> root_nodes(graph_info.passes.size(), true);
		{
			size_t current_pass_index = 0;
			for (const Vadon::Render::RenderPass& current_pass : graph_info.passes)
			{
				// Check whether we want to draw to something that has to be done by another pass first
				for (const Vadon::Render::PassResource& current_target : current_pass.targets)
				{
					auto target_it = resource_lookup.find(current_target.name);
					if (target_it != resource_lookup.end())
					{
						// Add to all passes that work on this target
						for (size_t parent_pass_index : target_it->second)
						{
							pass_child_lookup[parent_pass_index].push_back(current_pass_index);
						}
						root_nodes[current_pass_index] = false;
					}
				}

				// Check whether any of the input resources are the result of earlier passes
				for (const Vadon::Render::PassResource& current_resource : current_pass.resources)
				{
					auto resource_it = resource_lookup.find(current_resource.name);
					if (resource_it != resource_lookup.end())
					{
						// Add to all passes that work on this resource
						for (size_t parent_pass_index : resource_it->second)
						{
							pass_child_lookup[parent_pass_index].push_back(current_pass_index);
						}
						root_nodes[current_pass_index] = false;
					}
				}

				++current_pass_index;
			}

			for (auto& current_node_pair : pass_child_lookup)
			{
				// Sort and remove duplicates for good measure
				PassIndexVector& current_node_children = current_node_pair.second;
				std::sort(current_node_children.begin(), current_node_children.end());

				current_node_children.erase(std::unique(current_node_children.begin(), current_node_children.end()), current_node_children.end());
			}

			// TODO: make sure graph is DAG!!!
			FrameGraphHandle new_graph_handle = m_graph_pool.add();
			FrameGraph* new_frame_graph = m_graph_pool.get(new_graph_handle);

			// Store the node data
			new_frame_graph->nodes.reserve(graph_info.passes.size());
			for (const Vadon::Render::RenderPass& current_pass : graph_info.passes)
			{
				new_frame_graph->nodes.emplace_back(FrameGraphNode{ current_pass, Vadon::Utilities::DataRange() });
			}

			// Cache root nodes
			{
				size_t current_node_index = 0;
				for (bool is_root : root_nodes)
				{
					if (is_root)
					{
						new_frame_graph->root_nodes.push_back(current_node_index);
					}
					++current_node_index;
				}

				std::sort(new_frame_graph->root_nodes.begin(), new_frame_graph->root_nodes.end());
			}

			// Build the graph
			{
				std::vector<size_t> node_index_queue(new_frame_graph->root_nodes);
				size_t current_queue_index = 0;

				// Keep track of which nodes we added already
				std::vector<bool> visited_nodes(graph_info.passes.size(), false);

				while (current_queue_index < node_index_queue.size())
				{
					const size_t current_node_index = node_index_queue[current_queue_index];
					FrameGraphNode& current_node = new_frame_graph->nodes[current_node_index];

					auto pass_children_it = pass_child_lookup.find(current_node_index);
					if (pass_children_it != pass_child_lookup.end())
					{
						const PassIndexVector& current_node_children = pass_children_it->second;
						current_node.child_nodes.count = static_cast<int32_t>(current_node_children.size());
						current_node.child_nodes.offset = static_cast<int32_t>(new_frame_graph->child_indices.size());

						for (size_t current_child_index : current_node_children)
						{
							// Only add children who aren't added by other nodes
							if (!visited_nodes[current_child_index])
							{
								node_index_queue.push_back(current_child_index);
								new_frame_graph->child_indices.push_back(current_child_index);

								visited_nodes[current_child_index] = true;
							}
						}
					}

					++current_queue_index;
				}
			}

			return new_graph_handle;
		}
	}

	void FrameSystem::execute_graph(FrameGraphHandle graph_handle)
	{
		// FIXME: fit this into the task system?
		// We should be able to run render tasks in parallel with unrelated tasks, as long as render tasks themselves are kept in sequence
		FrameGraph* frame_graph = m_graph_pool.get(graph_handle);
	
		// Execute the graph breadth-first
		std::vector<size_t> node_index_queue(frame_graph->root_nodes);
		size_t current_queue_index = 0;

		while (current_queue_index < node_index_queue.size())
		{
			const size_t current_node_index = node_index_queue[current_queue_index];
			const FrameGraphNode& current_node = frame_graph->nodes[current_node_index];

			// Run the pass execution function
			current_node.pass.execution();

			// Add children to queue
			if (current_node.child_nodes.count > 0)
			{
				auto child_indices_begin = frame_graph->child_indices.begin() + current_node.child_nodes.offset;
				auto child_indices_end = child_indices_begin + current_node.child_nodes.count;

				node_index_queue.insert(node_index_queue.end(), child_indices_begin, child_indices_end);
			}

			++current_queue_index;
		}
	}

	void FrameSystem::remove_graph(FrameGraphHandle graph_handle)
	{
		m_graph_pool.remove(graph_handle);
	}

	FrameSystem::FrameSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Render::FrameSystem(core)
	{

	}

	bool FrameSystem::initialize()
	{
		// TODO!!!
		return true;
	}

	void FrameSystem::update()
	{
		// TODO!!!
	}

	void FrameSystem::shutdown()
	{
		// TODO!!!
	}
}