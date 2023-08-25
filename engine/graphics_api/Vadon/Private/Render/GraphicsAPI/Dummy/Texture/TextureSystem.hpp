#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_TEXTURE_TEXTURESYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_TEXTURE_TEXTURESYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Texture/TextureSystem.hpp>
namespace Vadon::Private::Render::Dummy
{
	class GraphicsAPI;

	class TextureSystem : public Vadon::Render::TextureSystem
	{
	public:
		Vadon::Render::TextureHandle create_2d_texture(const Vadon::Render::Texture2DInfo& texture_info, const Vadon::Render::ShaderResourceInfo& resource_info, const void* init_data = nullptr) override;

		Vadon::Render::ShaderResourceHandle get_texture_resource(Vadon::Render::TextureHandle texture_handle) override;
		void remove_texture(Vadon::Render::TextureHandle texture_handle) override;

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