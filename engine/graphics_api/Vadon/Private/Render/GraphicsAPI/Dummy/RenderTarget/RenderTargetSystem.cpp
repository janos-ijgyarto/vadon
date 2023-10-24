#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/Dummy/RenderTarget/RenderTargetSystem.hpp>

namespace Vadon::Private::Render::Dummy
{
	WindowHandle RenderTargetSystem::add_window(const WindowInfo& /*window_info*/)
	{
		// TODO
		return WindowHandle();
	}

	WindowInfo RenderTargetSystem::get_window_info(WindowHandle /*window_handle*/)
	{
		// TODO
		return WindowInfo();
	}

	void RenderTargetSystem::update_window(WindowHandle /*window_handle*/)
	{
		// TODO
	}

	void RenderTargetSystem::remove_window(WindowHandle /*window_handle*/)
	{
		// TODO
	}

	void RenderTargetSystem::resize_window(WindowHandle /*window_handle*/, const Utilities::Vector2i& /*window_size*/)
	{
		// TODO
	}

	void RenderTargetSystem::set_window_mode(WindowHandle /*window_handle*/, WindowMode /*mode*/)
	{
		// TODO
	}

	RenderTargetHandle RenderTargetSystem::add_target(const RenderTargetInfo& /*rt_info*/)
	{
		// TODO
		return RenderTargetHandle();
	}

	void RenderTargetSystem::copy_target(RenderTargetHandle /*source_handle*/, RenderTargetHandle /*destination_handle*/)
	{
		// TODO
	}

	void RenderTargetSystem::remove_target(RenderTargetHandle /*rt_handle*/)
	{
		// TODO
	}

	void RenderTargetSystem::set_target(RenderTargetHandle /*rt_handle*/, DepthStencilHandle /*ds_handle*/)
	{
		// TODO
	}

	void RenderTargetSystem::clear_target(RenderTargetHandle /*rt_handle*/, const Vadon::Render::RGBAColor& /*clear_color*/)
	{
		// TODO
	}

	void RenderTargetSystem::clear_depth_stencil(DepthStencilHandle /*ds_handle*/, const DepthStencilClear& /*clear*/)
	{
		// TODO
	}

	void RenderTargetSystem::remove_depth_stencil(DepthStencilHandle /*ds_handle*/)
	{
		// TODO
	}

	void RenderTargetSystem::apply_viewport(const Viewport& /*viewport*/)
	{
		// TODO
	}

	RenderTargetHandle RenderTargetSystem::get_window_target(WindowHandle /*window_handle*/)
	{
		// TODO
		return RenderTargetHandle();
	}

	RenderTargetSystem::RenderTargetSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::RenderTargetSystem(core)
		, m_graphics_api(graphics_api)
	{

	}
}