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

		const Resource* get_resource() const { return m_resource; }

		EntityModel& get_entity_model() { return m_entity_model; }

		static bool is_scene_base_of_type(VadonEditor::Core::Application& application, const QUuid& type_id);
		static QUuid get_scene_type_uuid();
		static QUuid get_scene_entities_uuid();

		bool save_scene() const;
		bool load_scene();
	private:
		Scene(Core::Application& application, Resource* resource);

		bool initialize();

		Core::Application& m_application;

		SceneID m_id;
		Resource* m_resource;
		EntityModel m_entity_model;

		friend class SceneSystem;
	};
}
#endif