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
	using SceneID = QUuid;

	class Entity : public QObject
	{
		Q_OBJECT
	public:
		Entity(Core::Application& application)
			: m_application(application)
			, m_data(application)
		{
		}

		~Entity();

		bool initialize();

		bool load_data(const QVariant& data);

		Core::Application& get_application() { return m_application; }

		QUuid get_id() const;
		QUuid get_parent() const;

		QString get_name() const;
		void set_name(const QString& name);

		QString get_label() const;

		SceneID get_sub_scene_id() const;

		Component* add_component(const QUuid& component_id);
		Component* get_component(const QUuid& component_id) const;
		void remove_component(const QUuid& component_id);

		QList<QUuid> get_component_id_list();
		void store_component_data();

		static QUuid components_property_uuid();
	signals:
		void name_changed(const QUuid& id);

		void component_added(const QUuid& entity_id, const QUuid& component_id);
		void component_removed(const QUuid& entity_id, const QUuid& component_id);
		void component_property_edited(const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id);
	private slots:
		void internal_component_property_edited(const QUuid& component_id, const QUuid& property_id);
	private:
		void internal_set_name(const QString& name);
		void set_id(const QUuid& id);

		void internal_add_component(Component* component);

		Core::Application& m_application;
		Core::DataObject m_data;

		QList<Component*> m_components;

		friend class EntityModel;
	};

	enum class EntityDataRole
	{
		ID = Qt::ItemDataRole::UserRole + 1
	};

	class EntityModel : public QObject
	{
		Q_OBJECT
	public:
		EntityModel(Core::Application& application);
		~EntityModel();

		QStandardItemModel& get_qt_model() { return m_qt_model; }
		Entity* get_root_entity() const { return m_root_entity; }

		Entity* get_entity_by_model_index(const QModelIndex& index) const;
		Entity* find_entity_by_id(const QUuid& id) const;
		QModelIndex find_entity_item_by_id(const QUuid& id) const;

		QUuid add_entity(Entity* parent);
		void remove_entity(const QUuid& id);

		bool save_data(QVariantList& entity_list) const;
		bool load_data(const QVariantList& entity_list);
	signals:
		void entity_added(const QUuid& id);
		void entity_removed(const QUuid& id);
		void entity_name_changed(const QUuid& id);

		void entity_component_added(const QUuid& entity_id, const QUuid& component_id);
		void entity_component_removed(const QUuid& entity_id, const QUuid& component_id);
		void entity_component_property_edited(const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id);
	private:
		Entity* internal_create_entity();
		Entity* internal_create_entity(QHash<QUuid, Entity*>& entity_lookup);

		void internal_add_entity(Entity* entity, QHash<QUuid, Entity*>& entity_lookup);
		static void clear_entity_lookup(QHash<QUuid, Entity*>& entity_lookup);

		void internal_create_root_entity();

		QStandardItem* internal_find_entity_item(const QUuid& id) const;
		bool save_entity_data_recursive(Entity* parent_entity, QVariantList& entity_list) const;

		void internal_remove_entity(QStandardItem* entity_item, const QUuid& id);

		void internal_entity_name_changed(const QUuid& id);
		void internal_entity_component_added(const QUuid& entity_id, const QUuid& component_id);
		void internal_entity_component_removed(const QUuid& entity_id, const QUuid& component_id);
		void internal_entity_component_property_edited(const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id);

		Core::Application& m_application;

		QHash<QUuid, Entity*> m_entity_lookup;
		QStandardItemModel m_qt_model;
		Entity* m_root_entity;
	};
}
#endif