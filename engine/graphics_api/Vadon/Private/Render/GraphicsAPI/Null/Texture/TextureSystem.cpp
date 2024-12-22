#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Null/Texture/TextureSystem.hpp>

namespace Vadon::Private::Render::Null
{
	Vadon::Render::TextureHandle TextureSystem::create_texture(const Vadon::Render::TextureInfo& /*texture_info*/, const void* /*init_data*/)
	{
		// TODO!!!
		return Vadon::Render::TextureHandle();
	}

	Vadon::Render::TextureHandle TextureSystem::create_texture_from_memory(size_t /*size*/, const void* /*data*/)
	{
		// TODO!!!
		return Vadon::Render::TextureHandle();
	}

	void TextureSystem::remove_texture(Vadon::Render::TextureHandle /*texture_handle*/)
	{
		// TODO!!!
	}

	Vadon::Render::TextureInfo TextureSystem::get_texture_info(Vadon::Render::TextureHandle /*texture_handle*/) const
	{
		// TODO!!!
		return Vadon::Render::TextureInfo();
	}

	Vadon::Render::ResourceViewHandle TextureSystem::create_resource_view(Vadon::Render::TextureHandle /*texture_handle*/, const Vadon::Render::TextureResourceViewInfo& /*resource_view_info*/)
	{
		// TODO!!!
		return Vadon::Render::ResourceViewHandle();
	}

	Vadon::Render::DepthStencilHandle TextureSystem::create_depth_stencil_view(Vadon::Render::TextureHandle /*texture_handle*/, const Vadon::Render::DepthStencilViewInfo& /*ds_view_info*/)
	{
		// TODO!!!
		return Vadon::Render::DepthStencilHandle();
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

	void TextureSystem::set_sampler_slots(Vadon::Render::ShaderType /*shader_type*/, const Vadon::Render::TextureSamplerSpan& /*samplers*/)
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