#ifndef VADONEDITOR_MODEL_SCENE_SCENETREE_HPP
#define VADONEDITOR_MODEL_SCENE_SCENETREE_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Entity;

	class SceneTree
	{
	public:
		Vadon::Scene::SceneHandle get_current_scene() const { return m_current_scene; }
		Entity* get_root() const { return m_current_scene_root; }

		// TODO: "archetypes" which automatically add certain components?
		// FIXME: use object pool so the raw pointers are not exposed?
		void add_entity(Entity* parent = nullptr);
		void remove_entity(Entity* entity);

		bool save_scene();
		bool load_scene();

		Vadon::Utilities::TypeInfoList get_component_type_list() const;
	private:
		SceneTree(Core::Editor& editor);

		bool initialize();
		Entity* instantiate_scene_recursive(Vadon::ECS::EntityHandle entity_handle, Entity* parent);

		Core::Editor& m_editor;

		Vadon::Scene::SceneHandle m_current_scene;
		Entity* m_current_scene_root = nullptr;

		friend class ModelSystem;
	};
}
#endif