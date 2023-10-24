#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_TEXTURE_TEXTURESYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_TEXTURE_TEXTURESYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Texture/TextureSystem.hpp>
namespace Vadon::Private::Render::Dummy
{
	class GraphicsAPI;

	class TextureSystem : public Vadon::Render::TextureSystem
	{
	public:
		Vadon::Render::TextureHandle create_texture(const Vadon::Render::TextureInfo& texture_info, const void* init_data = nullptr) override;
		Vadon::Render::TextureHandle create_texture_from_memory(size_t size, const void* data) override;
		void remove_texture(Vadon::Render::TextureHandle texture_handle) override;

		Vadon::Render::TextureInfo get_texture_info(Vadon::Render::TextureHandle texture_handle) const override;
		Vadon::Render::ResourceViewHandle create_resource_view(Vadon::Render::TextureHandle texture_handle, const Vadon::Render::ResourceViewInfo& resource_view_info) override;
		Vadon::Render::DepthStencilHandle create_depth_stencil_view(Vadon::Render::TextureHandle texture_handle, const Vadon::Render::DepthStencilViewInfo& ds_view_info) override;

		Vadon::Render::TextureSamplerHandle create_sampler(const Vadon::Render::TextureSamplerInfo& sampler_info) override;
		void remove_sampler(Vadon::Render::TextureSamplerHandle sampler_handle) override;

		void set_sampler(Vadon::Render::ShaderType shader_type, Vadon::Render::TextureSamplerHandle sampler_handle, int slot) override;
	protected:
		TextureSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void shutdown();

		GraphicsAPI& m_graphics_api;

		friend GraphicsAPI;
	};
}
#endif