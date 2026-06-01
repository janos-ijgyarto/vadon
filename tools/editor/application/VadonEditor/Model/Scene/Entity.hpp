#ifndef VADONEDITOR_MODEL_SCENE_ENTITY_HPP
#define VADONEDITOR_MODEL_SCENE_ENTITY_HPP
#include <VadonEditor/Model/Scene/Component.hpp>
#include <QList>
#include <QStandardItemModel>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class Scene;
	using SceneID = QUuid;

	class Entity
	{
	public:
		Entity(Core::Application& application)
			: m_application(application)
			, m_data(application)
		{
		}

		~Entity();

		bool initialize();

		bool load_entity_data(const QVariant& data);
		void store_entity_data();

		Core::Application& get_application() { return m_application; }

		QUuid get_id() const;
		QUuid get_parent() const;
		QString get_name() const;

		QString get_label() const;

		SceneID get_sub_scene_id() const;

		QList<QUuid> get_component_list() const;

		Component* add_component(const QUuid& type_id);
		Component* find_component(const QUuid& type_id);
		void remove_component(const QUuid& type_id);
	private:
		void internal_set_name(const QString& name);
		void set_id(const QUuid& id);

		void clear_components();

		Core::Application& m_application;

		Core::DataObject m_data;
		QHash<QUuid, Component*> m_components;

		bool m_modified = false;

		friend class EntityModel;
	};

	enum class EntityDataRole
	{
		ID = Qt::ItemDataRole::UserRole + 1
	};

	class EntityModel
	{
	public:
		EntityModel(Core::Application& application);
		~EntityModel();

		QStandardItemModel& get_qt_model() { return m_qt_model; }
		Entity* get_root_entity() const { return m_root_entity; }

		Entity* get_entity_by_model_index(const QModelIndex& index) const;
		Entity* find_entity_by_id(const QUuid& id) const;
		QModelIndex find_entity_item_by_id(const QUuid& id) const;

		void set_entity_name(Entity* entity, const QString& name);

		void remove_entity(const QUuid& id);

		bool save_data(QVariantList& entity_list) const;
		bool load_data(const QVariantList& entity_list);
	private:
		Entity* internal_create_entity();
		Entity* internal_create_entity(QHash<QUuid, Entity*>& entity_lookup);
		static void internal_add_entity(Entity* entity, QHash<QUuid, Entity*>& entity_lookup);
		static void clear_entity_lookup(QHash<QUuid, Entity*>& entity_lookup);

		QStandardItem* internal_find_entity_item(const QUuid& id) const;
		bool save_entity_data_recursive(Entity* parent_entity, QVariantList& entity_list) const;

		void internal_remove_entity(QStandardItem* entity_item, const QUuid& id);

		Core::Application& m_application;

		QHash<QUuid, Entity*> m_entity_lookup;
		QStandardItemModel m_qt_model;
		Entity* m_root_entity;
	};
}
#endif