#ifndef VADON_RENDER_GRAPHICSAPI_RENDERTARGET_RENDERTARGETSYSTEM_HPP
#define VADON_RENDER_GRAPHICSAPI_RENDERTARGET_RENDERTARGETSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>
namespace Vadon::Render
{
	using RGBAColor = Utilities::Vector4;

	class RenderTargetSystem : public GraphicsSystem<RenderTargetSystem>
	{
	public:
		// FIXME: make separate system for windows! RTs should be agnostic to swap chains, window management, etc.!
		// TODO: allow getting window back buffer as texture?
		virtual WindowHandle add_window(const WindowInfo& window_info) = 0;
		virtual bool is_window_valid(WindowHandle window_handle) const = 0;
		virtual WindowInfo get_window_info(WindowHandle window_handle) const = 0;
		virtual void update_window(WindowHandle window_handle) = 0;
		virtual void remove_window(WindowHandle window_handle) = 0;

		virtual void resize_window(WindowHandle window_handle, const Utilities::Vector2i& window_size) = 0;
		virtual void set_window_mode(WindowHandle window_handle, WindowMode mode) = 0;

		// FIXME: RT system just manages targets, they can be created from various resources, other systems will link to this!
		virtual RenderTargetHandle add_target(const RenderTargetInfo& rt_info) = 0;
		virtual bool is_render_target_valid(RenderTargetHandle rt_handle) const = 0;
		virtual void copy_target(RenderTargetHandle source_handle, RenderTargetHandle destination_handle) = 0;
		virtual void remove_target(RenderTargetHandle rt_handle) = 0;

		virtual void set_target(RenderTargetHandle rt_handle, DepthStencilHandle ds_handle) = 0;
		virtual void clear_target(RenderTargetHandle rt_handle, const RGBAColor& clear_color) = 0;
		virtual void clear_depth_stencil(DepthStencilHandle ds_handle, const DepthStencilClear& clear) = 0;

		virtual bool is_depth_stencil_valid(DepthStencilHandle ds_handle) const = 0;
		virtual void remove_depth_stencil(DepthStencilHandle ds_handle) = 0;

		virtual void apply_viewport(const Viewport& viewport) = 0;

		virtual RenderTargetHandle get_window_target(WindowHandle window_handle) const = 0;
	protected:
		RenderTargetSystem(Core::EngineCoreInterface& core) 
			: System(core) 
		{
		}
	};
}
#endif