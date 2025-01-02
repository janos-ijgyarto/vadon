#ifndef VADONEDITOR_RENDER_RENDERSYSTEM_HPP
#define VADONEDITOR_RENDER_RENDERSYSTEM_HPP
#include <VadonEditor/Render/Module.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/Window.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <memory>
namespace Vadon::Render::Canvas
{
	struct RenderContext;
}
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Render
{
	struct Shader
	{
		Vadon::Render::ShaderHandle vertex_shader;
		Vadon::Render::ShaderHandle pixel_shader;

		Vadon::Render::VertexLayoutHandle vertex_layout;
	};

	class RenderSystem : public RenderSystemBase<RenderSystem>
	{
	public:
		~RenderSystem();

		VADONEDITOR_API Vadon::Render::WindowHandle get_render_window() const;

		// FIXME: have a more elegant system?
		// Frame graphs should be constructed from viewports and RTs, render system just goes over any that need to be processed
		VADONEDITOR_API void enqueue_canvas(const Vadon::Render::Canvas::RenderContext& context);
	private:
		RenderSystem(Core::Editor& editor);

		bool initialize();
		void update();
		void shutdown();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Editor;
	};
}
#endif