#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_TEXTURE_TEXTURESYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_TEXTURE_TEXTURESYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Texture/TextureSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/RenderTarget/RenderTarget.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/SRV.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Shader.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Texture/Texture.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace Vadon::Private::Render::DirectX
{
	class GraphicsAPI;

	using TextureSamplerSpan = Vadon::Render::TextureSamplerSpan;

	class TextureSystem : public Vadon::Render::TextureSystem
	{
	public:
		TextureHandle create_texture(const TextureInfo& texture_info, const void* init_data = nullptr) override;
		TextureHandle create_texture_from_memory(size_t size, const void* data) override;
		bool is_texture_valid(TextureHandle texture_handle) const override;
		void remove_texture(TextureHandle texture_handle) override;

		TextureInfo get_texture_info(TextureHandle texture_handle) const override;

		SRVHandle create_shader_resource_view(TextureHandle texture_handle, const TextureSRVInfo& texture_srv_info) override;

		TextureSamplerHandle create_sampler(const TextureSamplerInfo& sampler_info) override;
		bool is_sampler_valid(TextureSamplerHandle sampler_handle) const override { return m_sampler_pool.is_handle_valid(sampler_handle); }
		void remove_sampler(TextureSamplerHandle sampler_handle) override;

		void set_sampler(ShaderType shader_type, TextureSamplerHandle sampler_handle, int slot) override;
		void set_sampler_slots(ShaderType shader_type, const TextureSamplerSpan& samplers) override;
	private:
		using SamplerPool = Vadon::Utilities::ObjectPool<Vadon::Render::TextureSampler, TextureSampler>;

		TextureSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void shutdown();

		D3DTextureHandle internal_create_texture(ResourceType type, D3DResource d3d_texture_resource);

		TextureHandle create_texture_from_dds(size_t size, const void* data);

		GraphicsAPI& m_graphics_api;
		SamplerPool m_sampler_pool;

		friend GraphicsAPI;
	};
}
#endif