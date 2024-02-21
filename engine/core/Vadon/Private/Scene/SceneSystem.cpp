#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Scene/SceneSystem.hpp>

#include <Vadon/Private/Core/Object/ObjectSystem.hpp>

#include <Vadon/Utilities/Data/VariantUtilities.hpp>
#include <Vadon/Utilities/Serialization/JSON/JSONImpl.hpp>

namespace Vadon::Private::Scene
{
	namespace
	{
		// FIXME: move to utility header
		template <typename T>
		concept IsTrivialJSONWrite = Vadon::Utilities::IsAnyOf<T, int, float, bool, std::string>;

		bool parse_scene_node(Node& node, int32_t parent_index, Vadon::Core::ObjectSystem& object_system, SceneData& scene_data)
		{
			const int32_t node_index = static_cast<int32_t>(scene_data.nodes.size());
			SceneData::NodeData& node_data = scene_data.nodes.emplace_back();

			node_data.name = node.get_name();
			node_data.type = node.get_class_id();
			node_data.parent = parent_index;

			const Core::ObjectPropertyList& node_properties = object_system.get_class_properties(node.get_class_id());
			for (const Core::ObjectProperty& current_property : node_properties)
			{
				SceneData::NodeData::Property& current_property_data = node_data.properties.emplace_back();

				current_property_data.name = current_property.name;
				current_property_data.value = object_system.get_property(node, current_property.name);
			}

			for (Node* current_child : node.get_children())
			{
				if (parse_scene_node(*current_child, node_index, object_system, scene_data) == false)
				{
					return false;
				}
			}

			return true;
		}

		void write_variant_to_json(Vadon::Utilities::JSON& json_object, const std::string& key, const Core::Variant& value)
		{			
			auto variant_visitor = Vadon::Utilities::VisitorOverloadList{
				[&](auto value)
				{
					if constexpr (IsTrivialJSONWrite<decltype(value)>)
					{
						json_object[key] = value;
					}
					else
					{
						// TODO: error?
					}
				}
				// TODO: other overloads?
			};

			std::visit(variant_visitor, value);
		}

		void read_json_to_variant(const Vadon::Utilities::JSON& json_value, Core::Variant& variant_value)
		{
			// FIXME: better way to do this?
			// FIXME2: support double separately?
			switch (json_value.type())
			{
				case Vadon::Utilities::JSONValueType::number_integer:
					variant_value = json_value.get<int>();
					break;
				case Vadon::Utilities::JSONValueType::number_float:
					variant_value = json_value.get<float>();
					break;
				case Vadon::Utilities::JSONValueType::boolean:
					variant_value = json_value.get<bool>();
					break;
				case Vadon::Utilities::JSONValueType::string:
					variant_value = json_value.get<std::string>();
					break;
					// TODO: default?
			}
		}
	}

	SceneHandle SceneSystem::create_scene(SceneInfo scene_info)
	{
		SceneHandle new_scene_handle = m_scene_pool.add();

		SceneData& scene_data = m_scene_pool.get(new_scene_handle);
		scene_data.info = scene_info;

		return new_scene_handle;
	}

	void SceneSystem::remove_scene(SceneHandle scene_handle)
	{
		// TODO: any cleanup?
		m_scene_pool.remove(scene_handle);
	}

	SceneInfo SceneSystem::get_scene_info(SceneHandle scene_handle) const
	{
		const SceneData& scene_data = m_scene_pool.get(scene_handle);
		return scene_data.info;
	}

	bool SceneSystem::set_scene_data(SceneHandle scene_handle, Node& root_node)
	{
		SceneData& scene_data = m_scene_pool.get(scene_handle);
		scene_data.clear();

		Vadon::Core::ObjectSystem& object_system = m_engine_core.get_system<Vadon::Core::ObjectSystem>();
		return parse_scene_node(root_node, 0, object_system, scene_data);
	}

	bool SceneSystem::save_scene(SceneHandle scene_handle, Vadon::Utilities::JSON& writer)
	{
		const SceneData& scene_data = m_scene_pool.get(scene_handle);

		// TODO: proper metadata (UID, etc.)!
		writer["type"] = "Scene";

		// FIXME: make this more efficient by emplacing the objects/arrays first?
		Vadon::Utilities::JSON nodes_array = Vadon::Utilities::JSON::array();
		for (const SceneData::NodeData& current_node_data : scene_data.nodes)
		{
			Vadon::Utilities::JSON current_node_object = Vadon::Utilities::JSON::object();

			current_node_object["name"] = current_node_data.name;
			current_node_object["parent"] = current_node_data.parent;
			current_node_object["type"] = current_node_data.type;
			
			Vadon::Utilities::JSON& node_properties_object = current_node_object["properties"];
			for (const SceneData::NodeData::Property& current_node_property : current_node_data.properties)
			{
				const std::string& property_name = current_node_property.name;
				write_variant_to_json(node_properties_object, property_name, current_node_property.value);
			}

			nodes_array.push_back(current_node_object);
		}

		writer["nodes"] = nodes_array;
		return true;
	}

	bool SceneSystem::load_scene(SceneHandle scene_handle, Vadon::Utilities::JSONReader& reader)
	{
		SceneData& original_scene_data = m_scene_pool.get(scene_handle);
		SceneData new_scene_data;

		if (reader.open_array("nodes") == false)
		{
			// TODO: error?
			return false;
		}

		const Vadon::Utilities::JSON& nodes_array = reader.get_current_object();
		for (const Vadon::Utilities::JSON& current_node_object : nodes_array)
		{
			SceneData::NodeData& node_data = new_scene_data.nodes.emplace_back();

			node_data.name = current_node_object["name"];
			node_data.parent = current_node_object["parent"];
			node_data.type = current_node_object["type"];

			if (reader.open_array("properties") == false)
			{
				// TODO: error?
				return false;
			}

			const Vadon::Utilities::JSON& node_properties_object = reader.get_current_object();
			for (const auto& current_property_object : node_properties_object.items())
			{
				SceneData::NodeData::Property& current_property = node_data.properties.emplace_back();

				current_property.name = current_property_object.key();

				const Vadon::Utilities::JSON& property_value = current_property_object.value().type();
				read_json_to_variant(property_value, current_property.value);
			}
			reader.close_object();
		}
		reader.close_object();

		// Everything succeeded, swap contents
		original_scene_data.swap(new_scene_data);

		return true;
	}

	void SceneSystem::update(float delta_time)
	{
		// FIXME: iterate tree once, gather nodes that need updates, place into linear queue, use until it changes
		// TODO: make a public "tree iterator" API?
		struct NodeStackEntry
		{
			Node* node = nullptr;
			size_t child_index = 0;
		};

		using NodeStack = std::vector<NodeStackEntry>;

		static NodeStack node_stack;
		{
			NodeStackEntry& root_entry = node_stack.emplace_back();
			root_entry.node = &m_root_node;

			m_root_node.update(delta_time);
		}

		while (node_stack.empty() == false)
		{
			NodeStackEntry& current_entry = node_stack.back();
			if (current_entry.child_index < current_entry.node->get_children().size())
			{
				// Update next child and add to stack
				NodeStackEntry child_entry;
				child_entry.node = current_entry.node->get_children()[current_entry.child_index];

				child_entry.node->update(delta_time);
				++current_entry.child_index;

				node_stack.push_back(child_entry);
			}
			else
			{
				node_stack.pop_back();
			}
		}
	}

	SceneSystem::SceneSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Scene::SceneSystem(core)
	{}

	bool SceneSystem::initialize()
	{
		Vadon::Core::ObjectSystem& object_system = m_engine_core.get_system<Vadon::Core::ObjectSystem>();
		object_system.register_object_class<Node>();
		// TODO: other node types?

		return true;
	}
}