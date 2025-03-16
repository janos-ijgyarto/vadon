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

		void init_entity(Vadon::ECS::EntityHandle entity);
		void update_entity(Vadon::ECS::EntityHandle entity);
		void remove_entity(Vadon::ECS::EntityHandle entity);

		void update_resource(VadonDemo::View::ViewResourceHandle resource_handle);
		void load_sprite_resource(VadonDemo::View::SpriteResourceHandle sprite_handle);

		void update_camera(VadonEditor::Model::Scene* active_scene);

		Core::Editor& m_editor;

		friend Core::Editor;
	};
}
#endif