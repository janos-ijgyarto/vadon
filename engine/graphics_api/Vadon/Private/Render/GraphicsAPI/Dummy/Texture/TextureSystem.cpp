#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Dummy/Texture/TextureSystem.hpp>

namespace Vadon::Private::Render::Dummy
{
	Vadon::Render::TextureHandle TextureSystem::create_2d_texture(const Vadon::Render::Texture2DInfo& /*texture_info*/, const Vadon::Render::ShaderResourceInfo& /*resource_info*/, const void* /*init_data*/)
	{
		// TODO!!!
		return Vadon::Render::TextureHandle();
	}

	Vadon::Render::ShaderResourceHandle TextureSystem::get_texture_resource(Vadon::Render::TextureHandle /*texture_handle*/)
	{
		// TODO!!!
		return Vadon::Render::ShaderResourceHandle();
	}

	void TextureSystem::remove_texture(Vadon::Render::TextureHandle /*texture_handle*/)
	{
		// TODO!!!
	}

	Vadon::Render::TextureSamplerHandle TextureSystem::create_sampler(const Vadon::Render::TextureSamplerInfo& /*sampler_info*/)
	{
		// TODO!!!
		return Vadon::Render::TextureSamplerHandle();
	}

	void TextureSystem::remove_sampler(Vadon::Render::TextureSamplerHandle /*sampler_handle*/)
	{
		// TODO!!!
	}

	void TextureSystem::set_sampler(Vadon::Render::ShaderType /*shader_type*/, Vadon::Render::TextureSamplerHandle /*sampler_handle*/, int /*slot*/)
	{
		// TODO!!!
	}

	TextureSystem::TextureSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::TextureSystem(core)
		, m_graphics_api(graphics_api)
	{
	}

	bool TextureSystem::initialize()
	{
		// TODO!!!
		return true;
	}

	void TextureSystem::shutdown()
	{
	}
}