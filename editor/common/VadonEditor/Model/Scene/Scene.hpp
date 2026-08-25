#ifndef VADONEDITOR_SCENE_SCENE_HPP
#define VADONEDITOR_SCENE_SCENE_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Model/Scene/Scene.hpp>
#include <Vadon/Foundation/Model/Scene/Scene.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace Vadon::Utilities
{
	class ObjectWrapper;
	class Serializer;
}
namespace VadonEditor::Model
{
	class Resource;

	class Scene
	{
	public:
		~Scene();

		Vadon::Model::SceneID get_id() const { return m_id; }
		Vadon::Model::SceneHandle get_handle() const { return m_handle; }

		bool is_loaded() const { return m_handle.is_valid() && m_root_entity.is_valid(); }

		Vadon::ECS::EntityHandle get_root_entity() const { return m_root_entity; }
	private:
		Scene(Core::Editor& editor, Vadon::Model::SceneID id, Resource* resource);

		bool initialize();
		void close_scene();

		void process_message(const char* data, size_t size);
		void process_entity_message(const ::Vadon::Foundation::UUID& entity_id, const char* data, size_t size);

		Vadon::ECS::EntityHandle internal_find_entity(const ::Vadon::Foundation::UUID& entity_id) const;
		void internal_add_entity(const ::Vadon::Foundation::SceneEntityInfo& entity_info);
		void internal_remove_entity(Vadon::ECS::EntityHandle entity_handle);

		bool load_component_property_data(Vadon::Utilities::Serializer& serializer, Vadon::Utilities::ObjectWrapper& component_wrapper, Vadon::ECS::EntityHandle owner_entity);

		void entity_added(Vadon::ECS::EntityHandle entity_handle);
		void entity_removed(Vadon::ECS::EntityHandle entity_handle);

		Core::Editor& m_editor;

		Vadon::Model::SceneID m_id;
		Vadon::Model::SceneHandle m_handle;
		Resource* m_resource;

		Vadon::ECS::EntityHandle m_root_entity;

		friend class SceneSystem;
	};
}
#endif