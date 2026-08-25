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
		VadonEditor::Model::Scene* get_active_scene() const { return m_active_scene; }
	private:
		EditorView(Core::Editor& editor);
		bool initialize();
        void update();

		void update_dirty_entities();

		void init_entity(Vadon::ECS::EntityHandle entity);
		void update_entity(Vadon::ECS::EntityHandle entity);
		void remove_entity(Vadon::ECS::EntityHandle entity);

		void resource_edited(Vadon::Model::ResourceID resource_id);
		void render_resource_edited(RenderResourceID view_render_resource);
		void texture_resource_edited(VadonDemo::Render::TextureResourceID texture_id);

		void load_render_resource(RenderResourceID view_render_resource);

		void update_camera();

		Core::Editor& m_editor;
		VadonEditor::Model::Scene* m_active_scene;

		friend Core::Editor;
	};
}
#endif