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
		virtual TextureHandle create_2d_texture(const Texture2DInfo& texture_info, const ShaderResourceInfo& resource_info, const void* init_data = nullptr) = 0;

		virtual ShaderResourceHandle get_texture_resource(TextureHandle texture_handle) = 0;
		virtual void remove_texture(TextureHandle texture_handle) = 0;

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