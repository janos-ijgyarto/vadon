#ifndef VADONEDITOR_MODEL_SCENE_SCENESYSTEM_HPP
#define VADONEDITOR_MODEL_SCENE_SCENESYSTEM_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <VadonEditor/Model/Scene/Event.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>
#include <functional>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class SceneSystem
	{
	public:
		using EntityEventCallback = std::function<void(const EntityEvent&)>;
		using ComponentEventCallback = std::function<void(const ComponentEvent&)>;

		~SceneSystem();

		VADONEDITOR_API const Scene* find_scene(const Vadon::Model::SceneID& scene_id) const;
		Scene* find_scene(const Vadon::Model::SceneID& scene_id) { return const_cast<Scene*>(std::as_const(*this).find_scene(scene_id)); }

		Scene* get_scene(const Vadon::Model::SceneID& scene_id);
		Scene* get_selected_scene() const { return m_selected_scene; }

		VADONEDITOR_API void add_entity_event_callback(EntityEventCallback callback);
		VADONEDITOR_API void add_component_event_callback(ComponentEventCallback callback);

		void dispatch_entity_event(const EntityEvent& event);
		void dispatch_component_event(const ComponentEvent& event);

		VADONEDITOR_API const Scene* find_entity_scene(Vadon::ECS::EntityHandle entity) const;
	private:
		SceneSystem(Core::Editor& editor);

		bool initialize();
		void shutdown();

		void process_message(const char* data, size_t size);
		void process_scene_message(const ::Vadon::Foundation::UUID& scene_id_uuid, const char* data, size_t size);

		void internal_remove_scene(Scene* scene);

		Core::Editor& m_editor;

		std::unordered_map<Vadon::Utilities::TypeUUID, Scene*> m_scene_lookup;
		Scene* m_selected_scene;

		std::vector<EntityEventCallback> m_entity_callbacks;
		std::vector<ComponentEventCallback> m_component_callbacks;

		friend Core::Editor;
	};
}
#endif