#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_GRAPHICSAPI_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_GRAPHICSAPI_HPP
#include <Vadon/Private/Render/GraphicsAPI/GraphicsAPI.hpp>

#include <Vadon/Private/Render/GraphicsAPI/Dummy/Buffer/BufferSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Dummy/Pipeline/PipelineSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Dummy/RenderTarget/RenderTargetSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Dummy/Shader/ShaderSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Dummy/Texture/TextureSystem.hpp>

namespace Vadon::Private::Render::Dummy
{
	class GraphicsAPI : public Vadon::Private::Render::GraphicsAPIBase
	{
	public:
		GraphicsAPI(Vadon::Core::EngineCoreInterface& core);

		void draw(const Vadon::Render::DrawCommand& command) override;

		bool initialize() override;
		void update() override;
		void shutdown() override;
	private:
		BufferSystem m_buffer_system;
		PipelineSystem m_pipeline_system;
		RenderTargetSystem m_rt_system;
		ShaderSystem m_shader_system;
		TextureSystem m_texture_system;
	};
}
#endif