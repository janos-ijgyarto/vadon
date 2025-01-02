#ifndef VADON_RENDER_GRAPHICSAPI_RENDERTARGET_RENDERTARGETSYSTEM_HPP
#define VADON_RENDER_GRAPHICSAPI_RENDERTARGET_RENDERTARGETSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/DSV.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/Window.hpp>
namespace Vadon::Render
{
	using RGBAColor = Utilities::Vector4;

	class RenderTargetSystem : public GraphicsSystem<RenderTargetSystem>
	{
	public:
		// FIXME: make separate system for windows! RTs should be agnostic to swap chains, window management, etc.!
		// TODO: allow getting window back buffer as texture?
		virtual WindowHandle create_window(const WindowInfo& window_info) = 0;
		virtual bool is_window_valid(WindowHandle window_handle) const = 0;
		virtual WindowInfo get_window_info(WindowHandle window_handle) const = 0;
		virtual void update_window(WindowHandle window_handle) = 0;
		virtual void remove_window(WindowHandle window_handle) = 0;

		virtual void resize_window(WindowHandle window_handle, const Utilities::Vector2i& window_size) = 0;

		virtual RTVHandle create_render_target_view(ResourceHandle resource_handle, const RenderTargetViewInfo& rtv_info) = 0;
		virtual bool is_rtv_valid(RTVHandle rtv_handle) const = 0;
		virtual void remove_render_target_view(RTVHandle rtv_handle) = 0;

		virtual DSVHandle create_depth_stencil_view(ResourceHandle resource_handle, const DepthStencilViewInfo& dsv_info) = 0;
		virtual bool is_dsv_valid(DSVHandle dsv_handle) const = 0;
		virtual void remove_depth_stencil_view(DSVHandle dsv_handle) = 0;

		virtual void set_target(RTVHandle rtv_handle, DSVHandle dsv_handle) = 0;
		virtual void clear_target(RTVHandle rtv_handle, const RGBAColor& clear_color) = 0;
		virtual void clear_depth_stencil(DSVHandle dsv_handle, const DepthStencilClear& clear) = 0;

		virtual void apply_viewport(const Viewport& viewport) = 0;

		virtual RTVHandle get_window_target(WindowHandle window_handle) const = 0;
	protected:
		RenderTargetSystem(Core::EngineCoreInterface& core) 
			: System(core) 
		{
		}
	};
}
#endif