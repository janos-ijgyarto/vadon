#ifndef VADONEDITOR_MODEL_SCENE_ENTITY_HPP
#define VADONEDITOR_MODEL_SCENE_ENTITY_HPP
#include <VadonEditor/Model/Scene/Component.hpp>
#include <QList>
#include <QStandardItemModel>
namespace VadonEditor::Model
{
	class Scene;
	using SceneID = QUuid;

	class Entity
	{
	public:
		~Entity();

		const QUuid& get_id() const { return m_id; }

		const QString& get_name() const { return m_name; }
		void set_name(const QString& name);

		QString get_label() const;

		Scene& get_owner_scene() { return m_owner_scene; }
		const SceneID& get_sub_scene_id() const { return m_sub_scene_id; }

		QHash<ComponentID, Component>& get_components() { return m_components; }

		QStandardItem* get_model_item() const;
	private:
		Entity(Scene& scene)
			: m_owner_scene(scene)
		{
		}

		bool save_data(QVariant& data, const QList<Entity*>& entity_queue) const;
		bool load_data(const QVariant& data, int& parent_index);

		QUuid m_id;
		Scene& m_owner_scene;

		QString m_name;

		QHash<ComponentID, Component> m_components;
		SceneID m_sub_scene_id;

		friend class EntityModel;
	};

	enum class EntityDataRole
	{
		ID = Qt::ItemDataRole::UserRole + 1
	};

	class EntityModel
	{
	public:
		EntityModel(Scene& owner_scene);
		~EntityModel();

		QStandardItemModel& get_qt_model() { return m_qt_model; }
		Entity* get_root_entity() const { return m_root_entity; }

		Entity* get_entity_by_model_index(const QModelIndex& index) const;
		QModelIndex find_entity_item_by_id(const QUuid& id) const;

		bool save_model(QVariantList& data) const;
		bool load_model(const QVariantList& data);

		void remove_entity(Entity* entity);
	private:
		Entity* internal_create_entity(QHash<QUuid, Entity*>& entity_lookup);
		static void internal_add_entity(Entity* entity, QHash<QUuid, Entity*>& entity_lookup);
		static void clear_entity_lookup(QHash<QUuid, Entity*>& entity_lookup);

		static void internal_delete_entity(Entity* entity);

		QHash<QUuid, Entity*> m_entity_lookup;
		QStandardItemModel m_qt_model;
		Scene& m_owner_scene;
		Entity* m_root_entity;
	};
}
#endif