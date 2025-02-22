#ifndef VADONDEMO_RENDER_RENDERSYSTEM_HPP
#define VADONDEMO_RENDER_RENDERSYSTEM_HPP
#include <VadonDemo/Render/Component.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Render/Frame/Graph.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/Window.hpp>
namespace Vadon::Render::Canvas
{
	struct RenderContext;
}
namespace VadonDemo::Core
{
	class GameCore;
}
namespace VadonDemo::Render
{
	struct Shader
	{
		Vadon::Render::ShaderHandle vertex_shader;
		Vadon::Render::ShaderHandle pixel_shader;

		Vadon::Render::VertexLayoutHandle vertex_layout;
	};

	class RenderSystem
	{
	public:
		~RenderSystem();

		Vadon::Render::Canvas::RenderContext& get_canvas_context();
		TextureResource* get_texture_resource(std::string_view path);
	private:
		RenderSystem(Core::GameCore& game_core);

		bool initialize();
		bool init_frame_graph();
		bool init_canvas_context();

		void pre_update(); // FIXME: temporary solution until ECS event fixes are implemented
		void update();
		void init_entity(Vadon::ECS::EntityHandle entity);
		void remove_entity(Vadon::ECS::EntityHandle entity);

		Core::GameCore& m_game_core;
		Vadon::Render::FrameGraphHandle m_frame_graph;

		Vadon::Render::WindowHandle m_render_window;

		CanvasContextHandle m_canvas_context;

		std::unordered_map<std::string, TextureResource> m_textures;

		std::vector<Vadon::ECS::EntityHandle> m_deferred_init_queue;

		friend Core::GameCore;
	};
}
#endif