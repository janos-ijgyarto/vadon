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

		void set_layers_dirty() { m_layers_dirty = true; }
		void update_dirty_layers();
		void update_editor_layer();

		Core::Editor& m_editor;
		std::unordered_map<const VadonEditor::Model::Scene*, CanvasContextHandle> m_scene_canvas_contexts;

		std::unordered_map<std::string, TextureResource> m_textures;

		Vadon::Render::Canvas::LayerHandle m_editor_layer;
		Vadon::Render::Canvas::ItemHandle m_editor_item;

		bool m_layers_dirty;

		friend Core::Editor;
	};
}
#endif