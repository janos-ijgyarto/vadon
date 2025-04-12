#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Null/RenderTarget/RenderTargetSystem.hpp>

namespace Vadon::Private::Render::Null
{
	WindowHandle RenderTargetSystem::create_window(const WindowInfo& /*window_info*/)
	{
		// TODO
		return WindowHandle();
	}

	WindowInfo RenderTargetSystem::get_window_info(WindowHandle /*window_handle*/) const
	{
		// TODO
		return WindowInfo();
	}

	WindowUpdateResult RenderTargetSystem::update_window(const WindowUpdateInfo& /*info*/)
	{
		// TODO
		return WindowUpdateResult::SUCCESSFUL;
	}

	void RenderTargetSystem::remove_window(WindowHandle /*window_handle*/)
	{
		// TODO
	}

	void RenderTargetSystem::resize_window(WindowHandle /*window_handle*/, const Utilities::Vector2i& /*window_size*/)
	{
		// TODO
	}

	Vadon::Render::RTVHandle RenderTargetSystem::create_render_target_view(Vadon::Render::ResourceHandle /*resource_handle*/, const Vadon::Render::RenderTargetViewInfo& /*rtv_info*/)
	{
		// TODO
		return Vadon::Render::RTVHandle();
	}

	void RenderTargetSystem::remove_render_target_view(Vadon::Render::RTVHandle /*rtv_handle*/)
	{
		// TODO
	}

	Vadon::Render::DSVHandle RenderTargetSystem::create_depth_stencil_view(Vadon::Render::ResourceHandle /*resource_handle*/, const Vadon::Render::DepthStencilViewInfo& /*dsv_info*/)
	{
		return Vadon::Render::DSVHandle();
	}

	void RenderTargetSystem::remove_depth_stencil_view(Vadon::Render::DSVHandle /*dsv_handle*/)
	{
		// TODO
	}

	void RenderTargetSystem::set_target(Vadon::Render::RTVHandle /*rtv_handle*/, Vadon::Render::DSVHandle /*dsv_handle*/)
	{
		// TODO
	}

	void RenderTargetSystem::clear_target(Vadon::Render::RTVHandle /*rtv_handle*/, const Vadon::Render::RGBAColor& /*clear_color*/)
	{
		// TODO
	}

	void RenderTargetSystem::clear_depth_stencil(Vadon::Render::DSVHandle /*dsv_handle*/, const Vadon::Render::DepthStencilClear& /*clear*/)
	{
		// TODO
	}

	void RenderTargetSystem::apply_viewport(const Vadon::Render::Viewport& /*viewport*/)
	{
		// TODO
	}

	Vadon::Render::RTVHandle RenderTargetSystem::get_window_back_buffer_view(Vadon::Render::WindowHandle /*window_handle*/)
	{
		// TODO
		return Vadon::Render::RTVHandle();
	}

	RenderTargetSystem::RenderTargetSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::RenderTargetSystem(core)
		, m_graphics_api(graphics_api)
	{

	}
}