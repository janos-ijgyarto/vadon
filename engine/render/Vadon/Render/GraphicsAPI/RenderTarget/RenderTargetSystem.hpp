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
		// TODO: allow getting window back buffer as texture?
		virtual WindowHandle add_window(const WindowInfo& window_info) = 0;
		virtual WindowInfo get_window_info(WindowHandle window_handle) = 0;
		virtual void update_window(WindowHandle window_handle) = 0;
		virtual void remove_window(WindowHandle window_handle) = 0;

		virtual void resize_window(WindowHandle window_handle, const Utilities::Vector2i& window_size) = 0;
		virtual void set_window_mode(WindowHandle window_handle, WindowMode mode) = 0;

		// TODO: add RT from texture (texture resource should be managed separately!)
		virtual RenderTargetHandle add_target(const RenderTargetInfo& rt_info) = 0;
		virtual void copy_target(RenderTargetHandle source_handle, RenderTargetHandle destination_handle) = 0;
		virtual void remove_target(RenderTargetHandle rt_handle) = 0;

		virtual void set_target(RenderTargetHandle rt_handle) = 0;
		virtual void clear_target(RenderTargetHandle rt_handle, const RGBAColor& clear_color) = 0;

		virtual void apply_viewport(const Viewport& viewport) = 0;

		virtual RenderTargetHandle get_window_target(WindowHandle window_handle) = 0;
	protected:
		RenderTargetSystem(Core::EngineCoreInterface& core) 
			: EngineSystem(core) 
		{
		}
	};
}
#endif