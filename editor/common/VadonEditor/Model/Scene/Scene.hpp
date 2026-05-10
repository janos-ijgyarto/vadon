#ifndef VADONEDITOR_SCENE_SCENE_HPP
#define VADONEDITOR_SCENE_SCENE_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Model/Scene/Scene.hpp>
namespace VadonEditor::Core
{
	struct Property;
}
namespace VadonEditor::Model
{
	class Scene
	{
	public:
		bool open();
		void close();

		Vadon::Model::SceneID get_id() const { return m_id; }
		Vadon::Model::SceneHandle get_handle() const { return m_handle; }

		bool is_loaded() const { return m_handle.is_valid(); }

		Vadon::ECS::EntityHandle get_root_entity() const { return m_root_entity; }
	private:
		Vadon::Model::SceneID m_id;
		Vadon::Model::SceneHandle m_handle;

		Vadon::ECS::EntityHandle m_root_entity;
	};
}
#endif