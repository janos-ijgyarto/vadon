#include <VadonEditor/Model/Scene/Entity.hpp>

#include <VadonEditor/Core/Data/Type.hpp>

#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/Utilities/UUID.hpp>
#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <Vadon/Foundation/Model/Scene/Scene.hpp>

namespace
{
	QVariant get_entity_item_data(const QStandardItem* item, VadonEditor::Model::EntityDataRole role)
	{
		return item->data(static_cast<int>(role));
	}

	void set_entity_item_data(QStandardItem* item, const QVariant& data, VadonEditor::Model::EntityDataRole role)
	{
		item->setData(data, static_cast<int>(role));
	}

	QStandardItem* create_scene_tree_standard_item(const QString& label, const QUuid& entity_id)
	{
		QStandardItem* new_item = new QStandardItem(label);
		new_item->setCheckable(false);
		new_item->setEditable(false);
		new_item->setDragEnabled(false);

		set_entity_item_data(new_item, entity_id, VadonEditor::Model::EntityDataRole::ID);

		return new_item;
	}

	QStandardItem* find_entity_by_id_recursive(const QStandardItem* parent, const QUuid& id)
	{
		for (int child_index = 0; child_index < parent->rowCount(); ++child_index)
		{
			QStandardItem* current_child = parent->child(child_index);
			if (get_entity_item_data(current_child, VadonEditor::Model::EntityDataRole::ID).toUuid() == id)
			{
				return current_child;
			}

			QStandardItem* found_descendant = find_entity_by_id_recursive(current_child, id);
			if (found_descendant != nullptr)
			{
				return found_descendant;
			}
		}

		return nullptr;
	}
}

namespace VadonEditor::Model
{
	Entity::~Entity()
	{
		Q_ASSERT_X(m_id.isNull() == true, "VadonEditor::Model::Entity::~Entity", "Attempted to delete Entity object outside Model");
	}

	void Entity::set_name(const QString& name)
	{
		m_name = name;
		get_model_item()->setText(get_label());
	}

	QString Entity::get_label() const
	{
		if (m_name.isEmpty() == false)
		{
			return m_name;
		}
		else
		{
			return QString("Entity_%1").arg(Utilities::uuid_to_base64_string(m_id));
		}
	}

	QStandardItem* Entity::get_model_item() const
	{
		EntityModel& entity_model = m_owner_scene.get_entity_model();
		
		const QModelIndex model_index = entity_model.find_entity_item_by_id(m_id);
		if (model_index.isValid() == true)
		{
			return entity_model.get_qt_model().itemFromIndex(model_index);
		}

		return nullptr;
	}

	Component* Entity::add_component(const QUuid& type_id)
	{
		auto component_it = m_components.find(type_id);
		if (component_it != m_components.end())
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Entity::add_component", "Component already added!");
			return &component_it.value();
		}
		
		Component new_component;
		new_component.type_id = type_id;

		if (new_component.initialize(m_owner_scene.get_application()) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Entity::add_component", "Failed to initialize component");
			return nullptr;
		}

		component_it = m_components.insert(type_id, new_component);
		return &component_it.value();
	}

	Component* Entity::find_component(const QUuid& type_id)
	{
		auto component_it = m_components.find(type_id);
		if (component_it != m_components.end())
		{
			return &component_it.value();
		}

		return nullptr;
	}

	void Entity::remove_component(const QUuid& type_id)
	{
		auto component_it = m_components.find(type_id);
		if (component_it == m_components.end())
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Entity::remove_component", "Component not found");
			return;
		}

		m_components.erase(component_it);
	}

	bool Entity::save_data(QVariant& data, const QList<Entity*>& entity_queue) const
	{
		QVariantMap entity_variant_map;

		{
			const QUuid id_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_id_property.id);
			entity_variant_map.insert(Utilities::serialize_labeled_uuid("id", id_property_uuid), Core::TypeData::serialize_base_type(::Vadon::Foundation::BaseType::UUID, m_id));

			const QUuid name_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_name_property.id);
			entity_variant_map.insert(Utilities::serialize_labeled_uuid("name", name_property_uuid), m_name);
		}

		QStandardItem* model_item = get_model_item();
		const QUuid parent_uuid = model_item->parent() ? get_entity_item_data(model_item->parent(), EntityDataRole::ID).toUuid() : QUuid();

		if (parent_uuid.isNull() == false)
		{
			int parent_index = -1;
			for (int current_index = 0; current_index < entity_queue.size(); ++current_index)
			{
				if (entity_queue[current_index]->get_id() == parent_uuid)
				{
					parent_index = current_index;
					break;
				}
			}
			if (parent_index == -1)
			{
				Q_ASSERT_X(false, "VadonEditor::Model::Entity::save_data", "Cannot find parent in queue");
				return false;
			}

			const QUuid parent_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_parent_property.id);
			entity_variant_map.insert(Utilities::serialize_labeled_uuid("parent", parent_property_uuid), parent_index);
		}

		if (m_sub_scene_id.isNull() == false)
		{
			const QUuid scene_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_scene_property.id);

			entity_variant_map.insert(Utilities::serialize_labeled_uuid("scene", scene_property_uuid), Core::TypeData::serialize_base_type(::Vadon::Foundation::BaseType::UUID, m_sub_scene_id));
		}

		{
			QVariantList component_data_list;
			for (auto component_it = m_components.begin(); component_it != m_components.end(); ++component_it)
			{
				QVariant component_data;
				const Component& current_component = component_it.value();
				if (current_component.save_data(m_owner_scene.get_application(), component_data) == false)
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

	bool Entity::load_data(const QVariant& data, int& parent_index)
	{
		const QVariantMap entity_variant_map = data.toMap();

		const QUuid id_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_id_property.id);
		const QUuid name_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_name_property.id);
		const QUuid parent_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_parent_property.id);
		const QUuid scene_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_scene_property.id);
		const QUuid components_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_components_property.id);

		for (auto entry_it = entity_variant_map.begin(); entry_it != entity_variant_map.end(); ++entry_it)
		{
			const QUuid entry_uuid = Utilities::parse_labeled_uuid(entry_it.key());

			if (entry_uuid == id_property_uuid)
			{
				m_id = Core::TypeData::deserialize_base_type(::Vadon::Foundation::BaseType::UUID, entry_it.value()).toUuid();
			}
			if (entry_uuid == name_property_uuid)
			{
				m_name = entry_it.value().toString();
			}
			else if (entry_uuid == parent_property_uuid)
			{
				parent_index = entry_it.value().toInt();
			}
			else if (entry_uuid == scene_property_uuid)
			{
				m_sub_scene_id = Core::TypeData::deserialize_base_type(::Vadon::Foundation::BaseType::UUID, entry_it.value()).toUuid();
			}
			else if (entry_uuid == components_property_id)
			{
				const QVariantList component_data_list = entry_it.value().toList();
				for (const QVariant& current_component_data : component_data_list)
				{
					Component current_component;
					if (current_component.load_data(m_owner_scene.get_application(), current_component_data) == false)
					{
						Q_ASSERT_X(false, "VadonEditor::Model::Entity::load_data", "Failed to load component data");
						return false;
					}

					if (m_components.find(current_component.type_id) != m_components.end())
					{
						Q_ASSERT_X(false, "VadonEditor::Model::Entity::load_data", "Component type already loaded");
						return false;
					}

					m_components.insert(current_component.type_id, current_component);
				}
			}
			else
			{
				// TODO: warn about unrecognized UUID?
			}
		}

		return true;
	}

	EntityModel::EntityModel(Scene& owner_scene)
		: m_owner_scene(owner_scene)
	{
		m_root_entity = internal_create_entity(m_entity_lookup);
		m_root_entity->m_name = "Root";

		QStandardItem* root_item = create_scene_tree_standard_item(m_root_entity->get_label(), m_root_entity->get_id());
		m_qt_model.invisibleRootItem()->appendRow(root_item);
	}

	EntityModel::~EntityModel()
	{
		clear_entity_lookup(m_entity_lookup);

		m_entity_lookup.clear();
		m_qt_model.clear();
	}

	Entity* EntityModel::get_entity_by_model_index(const QModelIndex& index) const
	{
		if (m_qt_model.checkIndex(index) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::EntityModel::get_entity_by_model_index", "Invalid index");
			return nullptr;
		}

		QStandardItem* entity_item = m_qt_model.itemFromIndex(index);
		const QUuid entity_uuid = get_entity_item_data(entity_item, EntityDataRole::ID).toUuid();

		return find_entity_by_id(entity_uuid);
	}

	Entity* EntityModel::find_entity_by_id(const QUuid& id) const
	{
		auto entity_it = m_entity_lookup.find(id);
		if (entity_it != m_entity_lookup.end())
		{
			return entity_it.value();
		}

		return nullptr;
	}

	QModelIndex EntityModel::find_entity_item_by_id(const QUuid& id) const
	{
		QStandardItem* entity_item = find_entity_by_id_recursive(m_qt_model.invisibleRootItem(), id);
		if ((entity_item != nullptr) && (entity_item != m_qt_model.invisibleRootItem()))
		{
			return entity_item->index();
		}

		return QModelIndex();
	}

	bool EntityModel::save_model(QVariantList& data) const
	{
		QList<Entity*> entity_queue;
		entity_queue.push_back(m_root_entity);

		int queue_index = 0;
		while (queue_index < entity_queue.count())
		{
			Entity* current_entity = entity_queue[queue_index];
			QStandardItem* current_model_item = current_entity->get_model_item();

			if ((current_entity->m_sub_scene_id.isNull() == true) && (current_model_item->rowCount() > 0))
			{
				for (int row_index = 0; row_index < current_model_item->rowCount(); ++row_index)
				{
					const QUuid current_child_id = get_entity_item_data(current_model_item->child(row_index), EntityDataRole::ID).toUuid();
					
					auto child_it = m_entity_lookup.find(current_child_id);
					Q_ASSERT_X(child_it != m_entity_lookup.end(), "VadonEditor::Model::EntityModel::save_model", "Cannot find child item!");
					
					entity_queue.append(child_it.value());
				}
			}

			QVariant current_entity_data;
			if (current_entity->save_data(current_entity_data, entity_queue) == false)
			{
				return false;
			}

			data.push_back(current_entity_data);
			++queue_index;
		}

		return true;
	}

	bool EntityModel::load_model(const QVariantList& data)
	{
		// NOTE: don't use internal_create_entity because we're loading the UUID 
		Entity* new_root = new Entity(m_owner_scene);

		int parent_index = -1;
		if (new_root->load_data(data.front(), parent_index) == false)
		{
			delete new_root;
			return false;
		}

		QList<Entity*> entity_queue;
		entity_queue.push_back(new_root);

		QHash<QUuid, Entity*> new_entity_lookup;
		internal_add_entity(new_root, new_entity_lookup);

		QStandardItem* new_root_item = create_scene_tree_standard_item(new_root->get_label(), new_root->get_id());
	
		int entity_data_index = 1;
		while (entity_data_index < data.count())
		{
			const QVariant& current_entity_data = data[entity_data_index];
			Entity* new_entity = new Entity(m_owner_scene);
			parent_index = -1;
			const bool load_success = new_entity->load_data(current_entity_data, parent_index);
			if ((load_success == false) || (parent_index == -1))
			{
				delete new_entity;
				clear_entity_lookup(new_entity_lookup);
				delete new_root_item;
				return false;
			}

			QStandardItem* new_entity_item = create_scene_tree_standard_item(new_entity->get_label(), new_entity->get_id());

			Entity* parent_entity = entity_queue[parent_index];
			QStandardItem* parent_item = nullptr;
			if (parent_entity == new_root)
			{
				parent_item = new_root_item;
			}
			else
			{
				parent_item = find_entity_by_id_recursive(new_root_item, parent_entity->get_id());
				Q_ASSERT_X(parent_item != nullptr, "VadonEditor::Model::EntityModel::load_model", "Cannot find parent item");
			}

			parent_item->appendRow(new_entity_item);

			internal_add_entity(new_entity, new_entity_lookup);
			++entity_data_index;
		}

		// Everything succeeded, clear the previous model and set the new root
		m_qt_model.clear();
		m_qt_model.invisibleRootItem()->appendRow(new_root_item);

		m_root_entity = new_root;

		m_entity_lookup.swap(new_entity_lookup);
		clear_entity_lookup(new_entity_lookup);

		return true;
	}

	void EntityModel::remove_entity(Entity* entity)
	{
		if (entity->m_id.isNull() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::EntityModel::remove_entity", "Entity not properly removed!");
			return;
		}

		auto entity_it = m_entity_lookup.find(entity->get_id());
		Q_ASSERT_X(entity_it != m_entity_lookup.end(), "VadonEditor::Model::EntityModel::remove_entity", "Cannot find entity");
		m_entity_lookup.erase(entity_it);
	}

	Entity* EntityModel::internal_create_entity(QHash<QUuid, Entity*>& entity_lookup)
	{
		Entity* new_entity = new Entity(m_owner_scene);
		new_entity->m_id = QUuid::createUuid(); // TODO: make sure UUID is unique in the project?
		internal_add_entity(new_entity, entity_lookup);
		return new_entity;
	}

	void EntityModel::internal_add_entity(Entity* entity, QHash<QUuid, Entity*>& entity_lookup)
	{
		Q_ASSERT_X(entity->get_id().isNull() == false, "VadonEditor::Model::EntityModel::internal_add_entity", "Invalid entity");
		if (entity_lookup.find(entity->get_id()) != entity_lookup.end())
		{
			Q_ASSERT_X(false, "VadonEditor::Model::EntityModel::internal_add_entity", "Entity already added");
			return;
		}

		entity_lookup.insert(entity->get_id(), entity);
	}

	void EntityModel::clear_entity_lookup(QHash<QUuid, Entity*>& entity_lookup)
	{
		for (auto entity_it = entity_lookup.begin(); entity_it != entity_lookup.end(); ++entity_it)
		{
			Entity* current_entity = entity_it.value();
			internal_delete_entity(current_entity);
		}

		entity_lookup.clear();
	}

	void EntityModel::internal_delete_entity(Entity* entity)
	{
		// Unset ID to ensure that destructor does not assert
		entity->m_id = QUuid();
		delete entity;
	}
}