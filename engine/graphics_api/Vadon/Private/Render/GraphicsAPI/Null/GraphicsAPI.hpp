#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_GRAPHICSAPI_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_GRAPHICSAPI_HPP
#include <Vadon/Private/Render/GraphicsAPI/GraphicsAPI.hpp>

#include <Vadon/Private/Render/GraphicsAPI/Null/Buffer/BufferSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Null/Pipeline/PipelineSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Null/RenderTarget/RenderTargetSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Null/Resource/ResourceSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Null/Shader/ShaderSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Null/Texture/TextureSystem.hpp>

namespace Vadon::Private::Render::Null
{
	class GraphicsAPI : public Vadon::Private::Render::GraphicsAPIBase
	{
	public:
		GraphicsAPI(Vadon::Core::EngineCoreInterface& core);

		void draw(const Vadon::Render::DrawCommand& command) override;

		bool initialize() override;
		void shutdown() override;
	private:
		BufferSystem m_buffer_system;
		PipelineSystem m_pipeline_system;
		RenderTargetSystem m_rt_system;
		ResourceSystem m_resource_system;
		ShaderSystem m_shader_system;
		TextureSystem m_texture_system;
	};
}
#endif