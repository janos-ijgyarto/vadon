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

		void init_entity(Vadon::ECS::EntityHandle entity);
	private:
		RenderSystem(Core::GameCore& game_core);

		bool initialize();
		bool init_frame_graph();
		bool init_canvas_context();

		void update();
		void remove_entity(Vadon::ECS::EntityHandle entity);

		Core::GameCore& m_game_core;
		Vadon::Render::FrameGraphHandle m_frame_graph;

		Vadon::Render::WindowHandle m_render_window;

		CanvasContextHandle m_canvas_context;

		std::unordered_map<std::string, TextureResource> m_textures;

		friend Core::GameCore;
	};
}
#endif