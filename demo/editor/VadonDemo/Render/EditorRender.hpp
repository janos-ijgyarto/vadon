#ifndef VADONDEMO_RENDER_EDITORRENDER_HPP
#define VADONDEMO_RENDER_EDITORRENDER_HPP
#include <VadonDemo/Render/Component.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <unordered_map>
namespace VadonEditor::Model
{
	class Scene;
}
namespace VadonDemo::Core
{
	class Editor;
}
namespace Vadon::Render::Canvas
{
	struct RenderContext;
}
namespace VadonDemo::Render
{
	class EditorRender
	{
	public:
		CanvasContextHandle get_scene_canvas_context(const VadonEditor::Model::Scene* active_scene);

		TextureResource* get_texture_resource(std::string_view path);
		// TODO: unload unused resources!

		void init_entity(Vadon::ECS::EntityHandle entity);
	private:
		EditorRender(Core::Editor& editor);
		bool initialize();
		void update();

		void update_entity(Vadon::ECS::EntityHandle entity);
		void remove_entity(Vadon::ECS::EntityHandle entity);

		Core::Editor& m_editor;
		std::unordered_map<const VadonEditor::Model::Scene*, CanvasContextHandle> m_scene_canvas_contexts;

		std::unordered_map<std::string, TextureResource> m_textures;

		friend Core::Editor;
	};
}
#endif