#ifndef VADONDEMO_RENDER_RENDERSYSTEM_HPP
#define VADONDEMO_RENDER_RENDERSYSTEM_HPP
#include <Vadon/Render/Frame/Graph.hpp>
#include <memory>
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
		void set_frame_graph(const Vadon::Render::FrameGraphInfo& graph_info);
	private:
		RenderSystem(Core::GameCore& game_core);

		bool initialize();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::GameCore;
	};
}
#endif