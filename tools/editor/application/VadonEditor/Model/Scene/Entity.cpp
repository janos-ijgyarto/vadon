#include <VadonEditor/Model/Scene/Entity.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Data/Type.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

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
		for (Component* current_component : m_components)
		{
			delete current_component;
		}
	}

	bool Entity::initialize()
	{
		return m_data.default_initialize(Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_type_uuid));
	}
	
	bool Entity::load_data(const QVariant& data)
	{
		m_data.load_properties(data.toMap());
		return internal_load_data();
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

	void Entity::set_name(const QString& name)
	{
		const QUuid name_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_name_property.id);
		m_data.set_property(name_property_uuid, name);

		emit(name_changed(get_id()));
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

	Component* Entity::add_component(const QUuid& component_id)
	{
		if (get_component(component_id) != nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Entity::add_component", "Component already added");
			return nullptr;
		}

		Component* new_component = new Component(m_application);
		if (new_component->initialize(component_id) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Entity::add_component", "Failed to initialize component");
			delete new_component;
			return nullptr;
		}

		internal_add_component(new_component);

		emit(component_added(get_id(), component_id));
		return new_component;
	}

	Component* Entity::get_component(const QUuid& component_id) const
	{
		for (Component* current_component : m_components)
		{
			if (current_component->get_type_id() == component_id)
			{
				return current_component;
			}
		}

		return nullptr;
	}

	void Entity::remove_component(const QUuid& component_id)
	{
		Component* component_to_remove = get_component(component_id);
		if (component_to_remove == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Entity::remove_component", "Component not found");
			return;
		}

		m_components.removeOne(component_to_remove);
		delete component_to_remove;

		emit(component_removed(get_id(), component_id));
	}

	QList<QUuid> Entity::get_component_id_list() const
	{
		QList<QUuid> component_id_list;
		for (Component* current_component : m_components)
		{
			component_id_list.push_back(current_component->get_type_id());
		}

		return component_id_list;
	}

	void Entity::store_component_data()
	{
		QVariantList component_data_list;

		const Entity* sub_scene_entity = nullptr;
		const SceneID sub_scene_id = get_sub_scene_id();
		if (sub_scene_id.isNull() == false)
		{
			SceneSystem& scene_system = m_application.get_model_system().get_scene_system();
			Scene* sub_scene = scene_system.get_scene(sub_scene_id);

			sub_scene_entity = sub_scene->get_entity_model().get_root_entity();
		}

		for (Component* current_component : m_components)
		{
			QVariantMap component_data = current_component->export_data();
			if (sub_scene_entity != nullptr)
			{
				const Component* ref_component = sub_scene_entity->get_component(current_component->get_type_id());
				Q_ASSERT_X(ref_component != nullptr, "VadonEditor::Model::Entity::store_component_data", "Cannot find ref component");

				const QString properties_key_string = Utilities::uuid_to_base64_string(Core::DataObject::get_properties_property_uuid());
				auto properties_it = component_data.find(properties_key_string);
				QVariantMap component_properties = properties_it->toMap();

				for (auto property_entry_it = component_properties.begin(); property_entry_it != component_properties.end();)
				{
					const QVariant ref_value = ref_component->get_property(Utilities::base64_string_to_uuid(property_entry_it.key()));
					if (property_entry_it.value() == ref_value)
					{
						// Property value is the same, so we can remove it from the map
						property_entry_it = component_properties.erase(property_entry_it);
					}
					else
					{
						// Property is different from original, so we keep it
						++property_entry_it;
					}
				}

				if (component_properties.isEmpty())
				{
					// No difference from source scene, skip component
					continue;
				}

				component_data.insert(properties_key_string, component_properties);
			}

			component_data_list.push_back(component_data);
		}

		m_data.set_property(components_property_uuid(), component_data_list);
	}

	QUuid Entity::components_property_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_components_property.id);
	}

	void Entity::internal_component_property_edited(const QUuid& component_id, const QUuid& property_id)
	{
		emit(component_property_edited(get_id(), component_id, property_id));
	}

	bool Entity::internal_load_data()
	{
		const SceneID sub_scene_id = get_sub_scene_id();
		if (sub_scene_id.isNull() == false)
		{
			// First copy components from reference scene
			SceneSystem& scene_system = m_application.get_model_system().get_scene_system();
			Scene* sub_scene = scene_system.get_scene(sub_scene_id);

			const Entity* sub_scene_entity = sub_scene->get_entity_model().get_root_entity();
			for (const Component* ref_component : sub_scene_entity->m_components)
			{
				Component* copied_component = new Component(m_application);
				if (copied_component->import_data(ref_component->export_data()) == false)
				{
					Q_ASSERT_X(false, "VadonEditor::Model::EntityModel::internal_load_data", "Failed to import ref component data");
					delete copied_component;
					continue;
				}

				internal_add_component(copied_component);
			}
		}

		// Gather the component data for this entity
		const QVariantList component_data_list = m_data.get_property(components_property_uuid()).toList();
		for (const QVariant& component_data : component_data_list)
		{
			const QVariantMap component_obj_data = component_data.toMap();

			if (sub_scene_id.isNull() == true)
			{
				Component* new_component = new Component(m_application);
				if (new_component->import_data(component_obj_data) == false)
				{
					Q_ASSERT_X(false, "VadonEditor::Model::EntityModel::create_component_list", "Failed to create component object!");
					delete new_component;
					continue;
				}

				internal_add_component(new_component);
			}
			else
			{
				Core::DataObject component_data_obj(m_application);
				if (component_data_obj.import_data(component_obj_data) == false)
				{
					return false;
				}

				Component* component = get_component(component_data_obj.get_type_id());
				if (component == nullptr)
				{
					qWarning() << "Stale component type" << component_data_obj.get_type_id() << "in Entity" << get_id();
					continue;
				}

				component->import_properties(component_data_obj.get_property_map());
			}
		}

		return true;
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

	void Entity::internal_add_component(Component* component)
	{
		connect(component, &Component::property_edited, this, &Entity::internal_component_property_edited);

		m_components.push_back(component);

		// Sort components by UUID
		std::sort(m_components.begin(), m_components.end(),
			+[](const Model::Component* lhs, const Model::Component* rhs)
			{
				// FIXME: sort by metadata name instead?
				return lhs->get_type_id() < rhs->get_type_id();
			}
		);
	}

	EntityModel::EntityModel(Core::Application& application)
		: m_application(application)
	{
		internal_create_root_entity();
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

	QUuid EntityModel::add_entity(Entity* parent)
	{
		return internal_add_entity(parent, QUuid{});
	}

	void EntityModel::remove_entity(const QUuid& id)
	{
		const bool is_root = m_root_entity->get_id() == id;

		// FIXME: find a way to instead just remove the model item, and connect to signals to know which Entity objects to remove?
		QStandardItem* entity_item = internal_find_entity_item(id);
		Q_ASSERT_X(entity_item != nullptr, "VadonEditor::Model::EntityModel::remove_entity", "Cannot find entity");

		internal_remove_entity(entity_item, id);

		QStandardItem* parent_item = entity_item->parent();
		QModelIndex parent_index;
		if ((parent_item != nullptr) && (parent_item != m_qt_model.invisibleRootItem()))
		{
			parent_index = parent_item->index();
		}

		m_qt_model.removeRow(entity_item->row(), parent_index);

		emit(entity_removed(id));

		if(is_root == true)
		{
			// If we removed the root entity, we replace with a blank root entity
			internal_create_root_entity();
			emit(entity_added(m_root_entity->get_id()));
		}
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
		if (new_root_entity->load_data(entity_list.front()) == false)
		{
			return false;
		}

		internal_add_entity(new_root_entity, new_entity_lookup);
		QStandardItem* new_root_item = create_scene_tree_standard_item(new_root_entity->get_label(), new_root_entity->get_id());

		for (int entity_index = 1; entity_index < entity_list.size(); ++entity_index)
		{
			Entity* current_entity = internal_create_entity();
			if (current_entity->load_data(entity_list[entity_index]) == false)
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

		// Set new root entity (NOTE: clearing the lookup will already clear the previous Entity)
		m_root_entity = new_root_entity;

		m_entity_lookup.swap(new_entity_lookup);
		clear_entity_lookup(new_entity_lookup);

		m_qt_model.clear();
		m_qt_model.invisibleRootItem()->appendRow(new_root_item);

		return true;
	}

	QUuid EntityModel::instantiate_scene(const SceneID& scene_id, Entity* parent)
	{
		return internal_add_entity(parent, scene_id);
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

	QUuid EntityModel::internal_add_entity(Entity* parent, const QUuid& sub_scene_id)
	{
		Q_ASSERT_X(parent != nullptr, "VadonEditor::Model::EntityModel::internal_add_entity", "Must provide a valid parent");

		Entity* new_entity = internal_create_entity(m_entity_lookup);

		const QUuid entity_parent_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_parent_property.id);
		const QUuid parent_id = parent->get_id();
		new_entity->m_data.set_property(entity_parent_property_uuid, parent_id);

		if (sub_scene_id.isNull() == false)
		{
			// Set the sub-scene ID
			const QUuid entity_sub_scene_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneEntitySchema::c_scene_property.id);
			new_entity->m_data.set_property(entity_sub_scene_property_uuid, sub_scene_id);

			// We can simply copy the components from the original entity
			SceneSystem& scene_system = m_application.get_model_system().get_scene_system();
			Scene* sub_scene = scene_system.get_scene(sub_scene_id);

			const Entity* sub_scene_entity = sub_scene->get_entity_model().get_root_entity();
			new_entity->m_data.set_property(Entity::components_property_uuid(), sub_scene_entity->m_data.get_property(Entity::components_property_uuid()));

			if (new_entity->internal_load_data() == false)
			{
				Q_ASSERT_X(false, "VadonEditor::Model::EntityModel::internal_add_entity", "Failed to load Entity");
			}
		}

		const QUuid new_entity_id = new_entity->get_id();

		QStandardItem* new_entity_item = create_scene_tree_standard_item(new_entity->get_label(), new_entity_id);

		const QModelIndex parent_item_index = find_entity_item_by_id(parent_id);
		Q_ASSERT_X(parent_item_index.isValid() == true, "VadonEditor::Model::EntityModel::add_entity", "Cannot find parent item");

		QStandardItem* parent_item = m_qt_model.itemFromIndex(parent_item_index);
		parent_item->appendRow(new_entity_item);

		emit(entity_added(new_entity_id));

		return new_entity_id;
	}

	void EntityModel::internal_add_entity(Entity* entity, QHash<QUuid, Entity*>& entity_lookup)
	{
		Q_ASSERT_X(entity->get_id().isNull() == false, "VadonEditor::Model::EntityModel::internal_add_entity", "Invalid entity");
		if (entity_lookup.find(entity->get_id()) != entity_lookup.end())
		{
			Q_ASSERT_X(false, "VadonEditor::Model::EntityModel::internal_add_entity", "Entity already added");
			return;
		}

		QObject::connect(entity, &Entity::name_changed, this, &EntityModel::internal_entity_name_changed);

		QObject::connect(entity, &Entity::component_added, this, &EntityModel::internal_entity_component_added);
		QObject::connect(entity, &Entity::component_removed, this, &EntityModel::internal_entity_component_removed);
		QObject::connect(entity, &Entity::component_property_edited, this, &EntityModel::internal_entity_component_property_edited);

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

	void EntityModel::internal_create_root_entity()
	{
		m_root_entity = internal_create_entity(m_entity_lookup);
		m_root_entity->internal_set_name("Root");

		QStandardItem* root_item = create_scene_tree_standard_item(m_root_entity->get_label(), m_root_entity->get_id());
		m_qt_model.invisibleRootItem()->appendRow(root_item);
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
			const QUuid child_uuid = get_entity_item_data(current_child_item, EntityDataRole::ID).toUuid();

			internal_remove_entity(current_child_item, child_uuid);
		}
	}

	void EntityModel::internal_entity_name_changed(const QUuid& id)
	{
		internal_find_entity_item(id)->setText(find_entity_by_id(id)->get_label());
		emit(entity_name_changed(id));
	}

	void EntityModel::internal_entity_component_added(const QUuid& entity_id, const QUuid& component_id)
	{
		// TODO: represent in the QStandardItem somehow?
		emit(entity_component_added(entity_id, component_id));
	}

	void EntityModel::internal_entity_component_removed(const QUuid& entity_id, const QUuid& component_id)
	{
		// TODO: represent in the QStandardItem somehow?
		emit(entity_component_removed(entity_id, component_id));
	}

	void EntityModel::internal_entity_component_property_edited(const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id)
	{
		emit(entity_component_property_edited(entity_id, component_id, property_id));
	}
}