#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_TEXTURE_TEXTURESYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_TEXTURE_TEXTURESYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Texture/TextureSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/RenderTarget/RenderTarget.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Resource.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Shader.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Texture/Texture.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace Vadon::Private::Render::DirectX
{
	class GraphicsAPI;

	class TextureSystem : public Vadon::Render::TextureSystem
	{
	public:
		TextureHandle create_texture(const TextureInfo& texture_info, const void* init_data = nullptr) override;
		TextureHandle create_texture_from_memory(size_t size, const void* data) override;
		void remove_texture(TextureHandle texture_handle) override;

		TextureInfo get_texture_info(TextureHandle texture_handle) const override;
		ResourceViewHandle create_resource_view(TextureHandle texture_handle, const ResourceViewInfo& resource_view_info) override;
		DepthStencilHandle create_depth_stencil_view(TextureHandle texture_handle, const DepthStencilViewInfo& ds_view_info) override;

		TextureSamplerHandle create_sampler(const TextureSamplerInfo& sampler_info) override;
		void remove_sampler(TextureSamplerHandle sampler_handle) override;

		void set_sampler(ShaderType shader_type, TextureSamplerHandle sampler_handle, int slot) override;
	private:
		using TexturePool = Vadon::Utilities::ObjectPool<Vadon::Render::Texture, Texture>;
		using SamplerPool = Vadon::Utilities::ObjectPool<Vadon::Render::TextureSampler, TextureSampler>;

		TextureSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void shutdown();

		GraphicsAPI& m_graphics_api;
		TexturePool m_texture_pool;
		SamplerPool m_sampler_pool;

		friend GraphicsAPI;
	};
}
#endif