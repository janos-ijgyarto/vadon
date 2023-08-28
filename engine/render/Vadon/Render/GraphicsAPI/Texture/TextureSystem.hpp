#ifndef VADON_RENDER_GRAPHICSAPI_TEXTURE_TEXTURESYSTEM_HPP
#define VADON_RENDER_GRAPHICSAPI_TEXTURE_TEXTURESYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/GraphicsModule.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/Texture.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Resource.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
namespace Vadon::Render
{
	class TextureSystem : public GraphicsSystem<TextureSystem>
	{
	public:
		virtual TextureHandle create_texture(const TextureInfo& texture_info, const void* init_data = nullptr) = 0;
		virtual TextureHandle create_texture_from_memory(size_t size, const void* data) = 0;
		virtual void remove_texture(TextureHandle texture_handle) = 0;

		virtual TextureInfo get_texture_info(TextureHandle texture_handle) const = 0;

		virtual ShaderResourceViewHandle create_texture_srv(TextureHandle texture_handle, const ShaderResourceViewInfo& srv_info) = 0;

		virtual TextureSamplerHandle create_sampler(const TextureSamplerInfo& sampler_info) = 0;
		virtual void remove_sampler(TextureSamplerHandle sampler_handle) = 0;

		virtual void set_sampler(ShaderType shader_type, TextureSamplerHandle sampler_handle, int slot) = 0;
	protected:
		TextureSystem(Core::EngineCoreInterface& core) 
			: EngineSystem(core) 
		{
		}
	};
}
#endif