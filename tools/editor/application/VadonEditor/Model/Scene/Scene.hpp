#ifndef VADONEDITOR_MODEL_SCENE_SCENE_HPP
#define VADONEDITOR_MODEL_SCENE_SCENE_HPP
#include <VadonEditor/Model/Scene/Entity.hpp>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class Resource;

	// NOTE: Scene is a "wrapper" around the base Resource, saves scene contents in the resource data
	class Scene
	{
	public:
		~Scene();

		Core::Application& get_application() { return m_application; }

		const SceneID& get_id() const { return m_id; }

		Resource* get_resource() { return m_resource; }
		const Resource* get_resource() const { return m_resource; }

		const EntityModel& get_entity_model() const { return m_entity_model; }
		EntityModel& get_entity_model() { return m_entity_model; }

		bool is_modified() const;
		void notify_modified();

		static bool is_scene_base_of_type(VadonEditor::Core::Application& application, const QUuid& type_id);
		static QUuid get_scene_type_uuid();
		static QUuid get_scene_entities_uuid();

		void open_scene();

		bool save_scene() const;
		bool load_scene();

		QUuid instantiate_scene(const QUuid& scene_id, Entity* parent);
	private:
		Scene(Core::Application& application, Resource* resource);

		bool initialize();
		bool store_scene_data() const;

		void entity_added(const QUuid& id);
		void entity_removed(const QUuid& id);
		void entity_name_changed(const QUuid& id);

		void entity_component_added(const QUuid& entity_id, const QUuid& component_id);
		void entity_component_removed(const QUuid& entity_id, const QUuid& component_id);
		void entity_component_property_edited(const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id);

		bool is_sub_scene_acyclic(const QUuid& scene_id);

		void message_scene_opened(bool reload);

		Core::Application& m_application;

		SceneID m_id;
		Resource* m_resource;
		EntityModel m_entity_model;

		friend class SceneSystem;
	};
}
#endif