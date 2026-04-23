#include <VadonEditor/Model/Scene/Entity.hpp>

#include <VadonEditor/Core/Data/Type.hpp>

#include <VadonEditor/Utilities/UUID.hpp>
#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <Vadon/Foundation/Model/Scene/Scene.hpp>

namespace
{
	QStandardItem* create_scene_tree_standard_item(const QString& label)
	{
		QStandardItem* new_item = new QStandardItem(label);
		new_item->setCheckable(false);
		new_item->setEditable(false);
		new_item->setDragEnabled(false);

		return new_item;
	}

	void recursive_create_entity_items(QStandardItem* current_parent_item, VadonEditor::Model::Entity* current_entity)
	{
		QStandardItem* entity_item = create_scene_tree_standard_item(current_entity->name);
		current_parent_item->appendRow(entity_item);

		for (VadonEditor::Model::Entity* child_entity : current_entity->children)
		{
			recursive_create_entity_items(entity_item, child_entity);
		}
	}
}

namespace VadonEditor::Model
{
	Entity::~Entity()
	{
		internal_set_parent(nullptr);

		for (Entity* current_child : children)
		{
			// Unset parent so we don't have to perform redundant cleanup steps
			current_child->parent = nullptr;
			delete current_child;
		}

		children.clear();
	}

	bool Entity::save_data(Core::Application& application, QVariant& data, const QList<Entity*>& entity_queue) const
	{
		QVariantMap entity_variant_map;

		{
			const QUuid name_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_name_property.id);
			entity_variant_map.insert(Utilities::serialize_labeled_uuid("name", name_property_uuid), name);
		}

		if (parent != nullptr)
		{
			const int parent_index = entity_queue.indexOf(parent);
			if (parent_index == -1)
			{
				Q_ASSERT_X(false, "VadonEditor::Model::Entity::save_data", "Cannot find parent in queue");
				return false;
			}

			const QUuid parent_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_parent_property.id);
			entity_variant_map.insert(Utilities::serialize_labeled_uuid("parent", parent_property_uuid), parent_index);
		}

		if (sub_scene_id.isNull() == false)
		{
			const QUuid scene_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_scene_property.id);

			entity_variant_map.insert(Utilities::serialize_labeled_uuid("scene", scene_property_uuid), Core::TypeData::serialize_base_type(::Vadon::Foundation::BaseType::UUID, sub_scene_id));
		}

		{
			QVariantList component_data_list;
			for (auto component_it = components.begin(); component_it != components.end(); ++component_it)
			{
				QVariant component_data;
				const Component& current_component = component_it.value();
				if (current_component.save_data(application, component_data) == false)
				{
					Q_ASSERT_X(false, "VadonEditor::Model::Entity::save_data", "Failed to save component data");
					return false;
				}

				component_data_list.push_back(component_data);
			}

			const QUuid components_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_components_property.id);
			entity_variant_map.insert(Utilities::serialize_labeled_uuid("components", components_property_id), component_data_list);
		}

		data = entity_variant_map;
		return true;
	}

	bool Entity::load_data(Core::Application& application, const QVariant& data, const QList<Entity*>& entity_queue)
	{
		const QVariantMap entity_variant_map = data.toMap();

		const QUuid name_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_name_property.id);
		const QUuid parent_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_parent_property.id);
		const QUuid scene_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_scene_property.id);
		const QUuid components_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_components_property.id);

		for (auto entry_it = entity_variant_map.begin(); entry_it != entity_variant_map.end(); ++entry_it)
		{
			const QUuid entry_uuid = Utilities::parse_labeled_uuid(entry_it.key());

			if (entry_uuid == name_property_uuid)
			{
				name = entry_it.value().toString();
			}
			else if(entry_uuid == parent_property_uuid)
			{ 
				const int parent_index = entry_it.value().toInt();
				set_parent(entity_queue[parent_index]);
			}
			else if (entry_uuid == scene_property_uuid)
			{
				sub_scene_id = Core::TypeData::deserialize_base_type(::Vadon::Foundation::BaseType::UUID, entry_it.value()).toUuid();
			}
			else if (entry_uuid == components_property_id)
			{
				const QVariantList component_data_list = entry_it.value().toList();
				for (const QVariant& current_component_data : component_data_list)
				{
					Component current_component;
					if (current_component.load_data(application, current_component_data) == false)
					{
						Q_ASSERT_X(false, "VadonEditor::Model::Entity::load_data", "Failed to load component data");
						return false;
					}

					if (components.find(current_component.type_id) != components.end())
					{
						Q_ASSERT_X(false, "VadonEditor::Model::Entity::load_data", "Component type already loaded");
						return false;
					}

					components.insert(current_component.type_id, current_component);
				}
			}
			else
			{
				// TODO: warn about unrecognized UUID?
			}
		}

		return true;
	}

	void Entity::set_parent(Entity* new_parent)
	{
		internal_set_parent(new_parent);
		if (new_parent != nullptr)
		{
			new_parent->internal_add_child(this);
		}
	}

	void Entity::internal_set_parent(Entity* new_parent)
	{
		if (parent != nullptr)
		{
			// Remove from previous parent
			const qsizetype removed_count = parent->children.removeAll(this);
			if (removed_count != 1)
			{
				Q_ASSERT_X(false, "VadonEditor::Model::Entity::internal_set_parent", "Entity was not correctly present in parent");
			}
		}
		parent = new_parent;
	}

	void Entity::internal_add_child(Entity* new_child)
	{
		const qsizetype child_index = children.indexOf(new_child);
		if (child_index == -1)
		{
			children.push_back(new_child);
		}
		else
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Entity::internal_add_child", "Entity already child of parent");
		}
	}

	EntityModel::EntityModel(Scene& owner_scene)
		: m_owner_scene(owner_scene)
	{
		m_root_entity = new Entity(m_owner_scene);
		m_root_entity->name = "Root";

		rebuild_qt_model();
	}

	bool EntityModel::save_model(Core::Application& application, QVariantList& data) const
	{
		QList<Entity*> entity_queue;
		entity_queue.push_back(m_root_entity);

		int queue_index = 0;
		while (queue_index < entity_queue.count())
		{
			Entity* current_entity = entity_queue[queue_index];

			if ((current_entity->sub_scene_id.isNull() == true) && (current_entity->children.isEmpty() == false))
			{
				entity_queue.append(current_entity->children);
			}

			QVariant current_entity_data;
			if (current_entity->save_data(application, current_entity_data, entity_queue) == false)
			{
				return false;
			}

			data.push_back(current_entity_data);
			++queue_index;
		}

		return true;
	}

	bool EntityModel::load_model(Core::Application& application, const QVariantList& data)
	{
		Entity* new_root = new Entity(m_owner_scene);

		QList<Entity*> entity_queue;
		entity_queue.push_back(new_root);

		if (new_root->load_data(application, data.front(), entity_queue) == false)
		{
			delete new_root;
			return false;
		}

		int entity_data_index = 1;
		while (entity_data_index < data.count())
		{
			const QVariant& current_entity_data = data[entity_data_index];
			Entity* new_entity = new Entity(m_owner_scene);
			if (new_entity->load_data(application, current_entity_data, entity_queue) == false)
			{
				if (new_entity->parent == nullptr)
				{
					delete new_entity;
				}
				delete new_root;
				return false;
			}
			++entity_data_index;
		}

		Entity* prev_root = m_root_entity;
		m_root_entity = new_root;

		delete prev_root;

		rebuild_qt_model();
		return true;
	}

	void EntityModel::rebuild_qt_model()
	{
		m_qt_model.clear();

		recursive_create_entity_items(m_qt_model.invisibleRootItem(), m_root_entity);
	}
}