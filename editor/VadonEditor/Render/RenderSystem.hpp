#ifndef VADONEDITOR_RENDER_RENDERSYSTEM_HPP
#define VADONEDITOR_RENDER_RENDERSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <memory>
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

	class RenderSystem
	{
	public:
		~RenderSystem();
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