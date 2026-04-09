#include <VadonEditor/Model/Scene/Entity.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Model/Scene/Scene.hpp>

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
		owner_scene = nullptr;
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

			entity_variant_map.insert(Utilities::serialize_labeled_uuid("scene", scene_property_uuid), Utilities::uuid_to_base64_string(sub_scene_id));
		}

		{
			QVariantList component_data_list;
			for (auto component_it = components.begin(); component_it != components.end(); ++component_it)
			{
				QVariant component_data;
				const Component& current_component = component_it.value();
				if (current_component.save_data(application, component_it.key(), component_data) == false)
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
		// TODO: load self data first, then load children recursively
		Q_UNUSED(application);
		Q_UNUSED(data);
		Q_UNUSED(entity_queue);
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
			current_entity->save_data(application, current_entity_data, entity_queue);

			data.push_back(current_entity_data);
			++queue_index;
		}

		return false;
	}

	bool EntityModel::load_model(Core::Application& application, const QVariantList& data)
	{
		Entity* new_root = new Entity();
		
		QList<Entity*> entity_list;
		entity_list.push_back(new_root);

		int entity_data_index = 0;
		while (entity_data_index < data.count())
		{
			const QVariant& current_entity_data = data[entity_data_index];
			// TODO: load entity!
			Q_UNUSED(application);
			Q_UNUSED(current_entity_data);
		}

		Entity* prev_root = m_root_entity;
		m_root_entity = new_root;

		delete prev_root;

		return true;
	}
}