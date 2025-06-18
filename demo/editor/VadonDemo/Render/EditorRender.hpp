#ifndef VADONDEMO_RENDER_EDITORRENDER_HPP
#define VADONDEMO_RENDER_EDITORRENDER_HPP
#include <VadonDemo/Render/Component.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Render/Frame/Graph.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/Window.hpp>
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

		// TODO: unload unused resources!
		void init_entity(Vadon::ECS::EntityHandle entity);

		void load_texture_resource(TextureResourceHandle texture_handle);
		void load_shader_resource(ShaderResourceHandle shader_handle);
	private:
		EditorRender(Core::Editor& editor);

		bool initialize();
		bool init_frame_graph();

		bool project_loaded();

		void update();

		void update_entity(Vadon::ECS::EntityHandle entity);
		void remove_entity(Vadon::ECS::EntityHandle entity);

		void update_background_sprite_entity(Vadon::ECS::EntityHandle entity);
		void update_fullscreen_effect_entity(Vadon::ECS::EntityHandle entity);

		void set_layers_dirty() { m_layers_dirty = true; }
		void update_dirty_layers();
		void update_editor_layer();

		void process_platform_events();

		Core::Editor& m_editor;
		
		Vadon::Render::FrameGraphHandle m_frame_graph;
		Vadon::Render::WindowHandle m_render_window;
		
		std::unordered_map<const VadonEditor::Model::Scene*, CanvasContextHandle> m_scene_canvas_contexts;

		std::unordered_map<std::string, TextureResource> m_textures;

		Vadon::Render::Canvas::LayerHandle m_editor_layer;
		Vadon::Render::Canvas::ItemHandle m_editor_item;

		bool m_layers_dirty;

		friend Core::Editor;
	};
}
#endif