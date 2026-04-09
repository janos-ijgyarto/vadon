#ifndef VADONEDITOR_MODEL_SCENE_ENTITY_HPP
#define VADONEDITOR_MODEL_SCENE_ENTITY_HPP
#include <VadonEditor/Model/Scene/Component.hpp>
#include <QList>
#include <QStandardItemModel>
namespace VadonEditor::Model
{
	class Scene;
	using SceneID = QUuid;

	struct Entity
	{
		QString name;
		Entity* parent;
		QList<Entity*> children;
		QHash<ComponentID, Component> components;
		Scene* owner_scene;
		SceneID sub_scene_id;

		Entity()
			: parent(nullptr)
			, owner_scene(nullptr)
		{ }

		~Entity();

		bool save_data(Core::Application& application, QVariant& data, const QList<Entity*>& entity_queue) const;
		bool load_data(Core::Application& application, const QVariant& data, const QList<Entity*>& entity_queue);

		void set_parent(Entity* new_parent);
	private:
		void internal_set_parent(Entity* new_parent);
		void internal_add_child(Entity* new_child);
	};

	class EntityModel
	{
	public:
		QStandardItemModel& get_qt_model() { return m_qt_model; }
		Entity* get_root_entity() const { return m_root_entity; }

		bool save_model(Core::Application& application, QVariantList& data) const;
		bool load_model(Core::Application& application, const QVariantList& data);
	private:
		QStandardItemModel m_qt_model;
		Entity* m_root_entity;
	};
}
#endif