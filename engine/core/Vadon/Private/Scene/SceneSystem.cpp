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

			// FIXME: filter to properties that are intended to be serialized?
			const Core::ObjectPropertyList& node_properties = object_system.get_object_properties(node);
			for (const Core::ObjectProperty& current_property : node_properties)
			{
				SceneData::NodeData::Property& current_property_data = node_data.properties.emplace_back();

				current_property_data.name = current_property.name;
				current_property_data.value = current_property.value;
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
					if constexpr (std::is_same_v<decltype(value), Vadon::Utilities::Vector2> == true)
					{
						// FIXME: have a lookup for the keys!
						json_object[key] = Vadon::Utilities::JSON::array({ "Vector2", value.x, value.y });
					}
					// TODO: other overloads?
				}
			};

			std::visit(variant_visitor, value);
		}

		void read_json_to_variant(const Vadon::Utilities::JSON& json_value, Core::Variant& variant_value)
		{
			// FIXME: better way to do this?
			// FIXME2: support double separately?
			if (json_value.is_object() == true)
			{
				// TODO!!!
			}
			else if (json_value.is_array() == true)
			{
				// FIXME: proper lookup system!
				const std::string& type_string = json_value[0];
				if (type_string == "Vector2")
				{
					variant_value = Vadon::Utilities::Vector2(json_value[1].get<float>(), json_value[2].get<float>());
				}
			}
			else
			{
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

		std::string get_node_path(const SceneData& scene, int32_t node_index)
		{
			const SceneData::NodeData& node_data = scene.nodes[node_index];
			if (node_data.has_parent() == false)
			{
				return ".";
			}

			std::string path;

			const SceneData::NodeData* current_node_data = &node_data;
			while (current_node_data->has_parent() == true)
			{
				path = current_node_data->name + "/" + path;
				current_node_data = &scene.nodes[current_node_data->parent];
			}

			// Trim a final slash
			path.pop_back();

			return path;
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
		return parse_scene_node(root_node, -1, object_system, scene_data);
	}

	Node* SceneSystem::instantiate_scene(SceneHandle scene_handle)
	{
		const SceneData& scene_data = m_scene_pool.get(scene_handle);
		Vadon::Core::ObjectSystem& object_system = m_engine_core.get_system<Vadon::Core::ObjectSystem>();

		std::vector<Node*> node_lookup;

		for (const SceneData::NodeData& current_node_data : scene_data.nodes)
		{
			Vadon::Core::Object* current_node_object = object_system.create_object(current_node_data.type);
			if (current_node_object == nullptr)
			{
				// TODO: error?
				continue;
			}
			Node* current_node = object_system.get_object_as<Vadon::Scene::Node>(*current_node_object);
			if (current_node == nullptr)
			{
				// TODO: error?
				delete current_node_object;
				continue;
			}

			node_lookup.push_back(current_node);

			for (const SceneData::NodeData::Property& current_property_data : current_node_data.properties)
			{
				object_system.set_property(*current_node_object, current_property_data.name, current_property_data.value);
			}

			if (current_node_data.has_parent() == true)
			{
				Node* parent_node = node_lookup[current_node_data.parent];
				parent_node->add_child(current_node);
			}
		}

		return node_lookup.front();
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
			if (current_node_data.has_parent() == true)
			{
				current_node_object["parent"] = get_node_path(scene_data, current_node_data.parent);
			}
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

		std::unordered_map<std::string, int32_t> node_index_lookup;

		const Vadon::Utilities::JSON& nodes_array = reader.get_current_object();
		for (const Vadon::Utilities::JSON& current_node_object : nodes_array)
		{
			Vadon::Utilities::JSONReader current_node_reader(current_node_object);

			// FIXME: only add to array once we're sure everything is correct?
			const int32_t node_index = static_cast<int32_t>(new_scene_data.nodes.size());
			SceneData::NodeData& node_data = new_scene_data.nodes.emplace_back();

			node_data.name = current_node_object["name"];

			{
				auto parent_it = current_node_object.find("parent");
				if (parent_it != current_node_object.end())
				{
					std::string parent_path = parent_it.value();
					if (parent_path != ".")
					{
						parent_path = "./" + parent_path;
					}
					auto parent_index_it = node_index_lookup.find(parent_path);
					if (parent_index_it != node_index_lookup.end())
					{
						node_data.parent = parent_index_it->second;
					}
					else
					{
						// TODO: error?
						assert(false);
					}

					const std::string current_node_path = parent_path + "/" + node_data.name;
					node_index_lookup[current_node_path] = node_index;
				}
				else
				{
					// No parent, so this is the root
					node_index_lookup["."] = node_index;
				}
			}
			node_data.type = current_node_object["type"];

			if (current_node_reader.open_object("properties") == false)
			{
				// TODO: error?
				return false;
			}

			const Vadon::Utilities::JSON& node_properties_object = current_node_reader.get_current_object();
			for (const auto& current_property_object : node_properties_object.items())
			{
				SceneData::NodeData::Property& current_property = node_data.properties.emplace_back();

				current_property.name = current_property_object.key();

				const Vadon::Utilities::JSON& property_value = current_property_object.value();
				read_json_to_variant(property_value, current_property.value);
			}
			current_node_reader.close_object();
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