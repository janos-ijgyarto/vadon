#ifndef VADON_RENDER_GRAPHICSAPI_TEXTURE_TEXTURESYSTEM_HPP
#define VADON_RENDER_GRAPHICSAPI_TEXTURE_TEXTURESYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/GraphicsModule.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/Texture.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <span>
namespace Vadon::Render
{
	struct TextureSamplerSpan
	{
		int32_t start_slot = 0;
		std::span<TextureSamplerHandle> samplers;
	};

	class TextureSystem : public GraphicsSystem<TextureSystem>
	{
	public:
		virtual TextureHandle create_texture(const TextureInfo& texture_info, const void* init_data = nullptr) = 0;
		virtual TextureHandle create_texture_from_memory(size_t size, const void* data) = 0;
		virtual bool is_texture_valid(TextureHandle texture_handle) const = 0;
		virtual void remove_texture(TextureHandle texture_handle) = 0;

		virtual TextureInfo get_texture_info(TextureHandle texture_handle) const = 0;

		// FIXME: add function to create RT from texture!
		virtual ResourceViewHandle create_resource_view(TextureHandle texture_handle, const TextureResourceViewInfo& resource_view_info) = 0;
		virtual DepthStencilHandle create_depth_stencil_view(TextureHandle texture_handle, const DepthStencilViewInfo& ds_view_info) = 0;

		virtual TextureSamplerHandle create_sampler(const TextureSamplerInfo& sampler_info) = 0;
		virtual bool is_sampler_valid(TextureSamplerHandle sampler_handle) const = 0;
		virtual void remove_sampler(TextureSamplerHandle sampler_handle) = 0;

		virtual void set_sampler(ShaderType shader_type, TextureSamplerHandle sampler_handle, int32_t slot) = 0;
		virtual void set_sampler_slots(ShaderType shader_type, const TextureSamplerSpan& samplers) = 0;
	protected:
		TextureSystem(Core::EngineCoreInterface& core) 
			: EngineSystem(core) 
		{
		}
	};
}
#endif