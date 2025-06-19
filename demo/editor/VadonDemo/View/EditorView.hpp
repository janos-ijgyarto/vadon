#ifndef VADONDEMO_VIEW_EDITORVIEW_HPP
#define VADONDEMO_VIEW_EDITORVIEW_HPP
#include <VadonDemo/View/Resource.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
namespace VadonEditor::Model
{
	class Scene;
}
namespace VadonDemo::Core
{
	class Editor;
}
namespace VadonDemo::View
{
	class EditorView
	{
	public:
	private:
		EditorView(Core::Editor& editor);
		bool initialize();
        void update();

		void update_dirty_entities();

		void init_entity(Vadon::ECS::EntityHandle entity);
		void update_entity(Vadon::ECS::EntityHandle entity);
		void remove_entity(Vadon::ECS::EntityHandle entity);

		void resource_edited(Vadon::Scene::ResourceHandle resource_handle);
		void view_resource_edited(ViewResourceHandle view_resource);
		void texture_resource_edited(VadonDemo::Render::TextureResourceHandle texture_handle);

		void load_view_resource(ViewResourceHandle view_resource);

		void update_camera(VadonEditor::Model::Scene* active_scene);

		Core::Editor& m_editor;
		bool m_entities_dirty;

		friend Core::Editor;
	};
}
#endif