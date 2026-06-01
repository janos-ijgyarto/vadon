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
		clear_components();
	}

	bool Entity::initialize()
	{
		return m_data.initialize(Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_type_uuid));
	}
	
	bool Entity::load_entity_data(const QVariant& data)
	{
		clear_components();
		m_data.load_properties(data.toMap());

		// TODO: create component objects from data in the relevant property

		return true;
	}

	void Entity::store_entity_data()
	{
		// TODO: go over each component, gather data, write into array and add to the relevant property
	}

	QUuid Entity::get_id() const
	{
		const QUuid id_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_id_property.id);
		return m_data.get_property(id_property_uuid).toUuid();
	}

	QUuid Entity::get_parent() const
	{
		const QUuid parent_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_parent_property.id);
		return m_data.get_property(parent_property_uuid).toUuid();
	}

	QString Entity::get_name() const
	{
		const QUuid name_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_name_property.id);
		return m_data.get_property(name_property_uuid).toString();
	}

	QString Entity::get_label() const
	{
		const QString name = get_name();
		if (name.isEmpty() == false)
		{
			return name;
		}
		else
		{
			return QString("Entity_%1").arg(Utilities::uuid_to_base64_string(get_id()));
		}
	}

	SceneID Entity::get_sub_scene_id() const
	{
		const QUuid scene_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_scene_property.id);
		return m_data.get_property(scene_property_uuid).toUuid();
	}

	QList<QUuid> Entity::get_component_list() const
	{
		return QList<QUuid>();
	}

	Component* Entity::add_component(const QUuid& type_id)
	{
		auto component_it = m_components.find(type_id);
		if (component_it != m_components.end())
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Entity::add_component", "Component already added!");
			return component_it.value();
		}
		
		Component* new_component = new Component(m_application);
		if (new_component->initialize(type_id) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Entity::add_component", "Failed to initialize component");
			delete new_component;
			return nullptr;
		}

		component_it = m_components.insert(type_id, new_component);
		return component_it.value();
	}

	Component* Entity::find_component(const QUuid& type_id)
	{
		auto component_it = m_components.find(type_id);
		if (component_it != m_components.end())
		{
			return component_it.value();
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

	void Entity::internal_set_name(const QString& name)
	{
		const QUuid name_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_name_property.id);
		m_data.set_property(name_property_uuid, name);
	}

	void Entity::set_id(const QUuid& id)
	{
		const QUuid id_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_id_property.id);
		m_data.set_property(id_property_uuid, id);
	}

	void Entity::clear_components()
	{
		for (auto component_it = m_components.begin(); component_it != m_components.end(); ++component_it)
		{
			Component* current_component = component_it.value();
			delete current_component;
		}

		m_components.clear();
	}

	EntityModel::EntityModel(Core::Application& application)
		: m_application(application)
	{
		m_root_entity = internal_create_entity(m_entity_lookup);
		m_root_entity->internal_set_name("Root");

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
		QStandardItem* entity_item = internal_find_entity_item(id);
		if (entity_item != nullptr)
		{
			return entity_item->index();
		}

		return QModelIndex();
	}

	void EntityModel::set_entity_name(Entity* entity, const QString& name)
	{
		const QUuid name_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_name_property.id);
		entity->m_data.set_property(name_property_uuid, name);

		internal_find_entity_item(entity->get_id())->setText(entity->get_label());
	}

	void EntityModel::remove_entity(const QUuid& id)
	{
		// FIXME: find a way to instead just remove the model item, and connect to signals to know which Entity objects to remove?
		QStandardItem* entity_item = internal_find_entity_item(id);
		Q_ASSERT_X(entity_item != nullptr, "VadonEditor::Model::EntityModel::remove_entity", "Cannot find entity");

		internal_remove_entity(entity_item, id);

		delete entity_item;
	}

	bool EntityModel::save_data(QVariantList& entity_list) const
	{
		if (save_entity_data_recursive(m_root_entity, entity_list) == false)
		{
			return false;
		}

		return true;
	}

	bool EntityModel::load_data(const QVariantList& entity_list)
	{
		QHash<QUuid, Entity*> new_entity_lookup;
		Entity* new_root_entity = internal_create_entity();
		if (new_root_entity->load_entity_data(entity_list.front()) == false)
		{
			return false;
		}

		internal_add_entity(new_root_entity, new_entity_lookup);
		QStandardItem* new_root_item = create_scene_tree_standard_item(new_root_entity->get_label(), new_root_entity->get_id());

		for (int entity_index = 1; entity_index < entity_list.size(); ++entity_index)
		{
			Entity* current_entity = internal_create_entity();
			if (current_entity->load_entity_data(entity_list[entity_index]) == false)
			{
				clear_entity_lookup(new_entity_lookup);
				delete new_root_item;
				return false;
			}

			internal_add_entity(current_entity, new_entity_lookup);

			const QUuid parent_id = current_entity->get_parent();
			QStandardItem* current_entity_item = create_scene_tree_standard_item(current_entity->get_label(), current_entity->get_id());
			if (parent_id == new_root_entity->get_id())
			{
				new_root_item->appendRow(current_entity_item);
			}
			else
			{
				QStandardItem* parent_item = find_entity_by_id_recursive(new_root_item, parent_id);
				Q_ASSERT_X(parent_item != nullptr, "VadonEditor::Model::EntityModel::load_data", "Cannot find parent");

				parent_item->appendRow(current_entity_item);
			}
		}

		Entity* prev_root = m_root_entity;
		m_root_entity = new_root_entity;

		delete prev_root;

		m_entity_lookup.swap(new_entity_lookup);
		clear_entity_lookup(new_entity_lookup);

		m_qt_model.clear();
		m_qt_model.invisibleRootItem()->appendRow(new_root_item);

		return true;
	}

	Entity* EntityModel::internal_create_entity()
	{
		VadonEditor::Model::Entity* entity = new VadonEditor::Model::Entity(m_application);
		if (entity->initialize() == false)
		{
			delete entity;
			return nullptr;
		}

		return entity;
	}

	Entity* EntityModel::internal_create_entity(QHash<QUuid, Entity*>& entity_lookup)
	{
		Entity* new_entity = internal_create_entity();
		new_entity->set_id(QUuid::createUuid()); // TODO: make sure UUID is unique in the project?
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
			delete current_entity;
		}

		entity_lookup.clear();
	}

	QStandardItem* EntityModel::internal_find_entity_item(const QUuid& id) const
	{
		QStandardItem* entity_item = find_entity_by_id_recursive(m_qt_model.invisibleRootItem(), id);
		if ((entity_item != nullptr) && (entity_item != m_qt_model.invisibleRootItem()))
		{
			return entity_item;
		}

		return nullptr;
	}

	bool EntityModel::save_entity_data_recursive(Entity* parent_entity, QVariantList& entity_list) const
	{
		// First add self to the list
		entity_list.append(parent_entity->m_data.get_property_map());

		// Recursively add all children
		QStandardItem* entity_item = internal_find_entity_item(parent_entity->get_id());
		for (int child_index = 0; child_index < entity_item->rowCount(); ++child_index)
		{
			QStandardItem* child_item = entity_item->child(child_index);
			Entity* child_entity = get_entity_by_model_index(child_item->index());
			if (save_entity_data_recursive(child_entity, entity_list) == false)
			{
				return false;
			}
		}

		return true;
	}

	void EntityModel::internal_remove_entity(QStandardItem* entity_item, const QUuid& id)
	{
		auto entity_it = m_entity_lookup.find(id);
		Q_ASSERT_X(entity_it != m_entity_lookup.end(), "VadonEditor::Model::EntityModel::remove_entity", "Cannot find entity");

		delete entity_it.value();

		m_entity_lookup.erase(entity_it);

		for (int child_index = 0; child_index < entity_item->rowCount(); ++child_index)
		{
			QStandardItem* current_child_item = entity_item->child(child_index);
			const QUuid child_uuid = get_entity_item_data(entity_item, EntityDataRole::ID).toUuid();

			internal_remove_entity(current_child_item, child_uuid);
		}
	}
}