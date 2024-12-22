#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_RENDERTARGET_RENDERTARGETSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_RENDERTARGET_RENDERTARGETSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

#include <Vadon/Private/Render/GraphicsAPI/Null/RenderTarget/RenderTarget.hpp>
namespace Vadon::Private::Render::Null
{
	class GraphicsAPI;

	class RenderTargetSystem : public Vadon::Render::RenderTargetSystem
	{
	public:
		WindowHandle add_window(const WindowInfo& window_info) override;
		bool is_window_valid(WindowHandle /*window_handle*/) const override { return false; }
		WindowInfo get_window_info(WindowHandle window_handle) const override;
		void update_window(WindowHandle window_handle) override;
		void remove_window(WindowHandle window_handle) override;

		void resize_window(WindowHandle window_handle, const Utilities::Vector2i& window_size) override;
		void set_window_mode(WindowHandle window_handle, WindowMode mode) override;

		RenderTargetHandle add_target(const RenderTargetInfo& rt_info) override;
		bool is_render_target_valid(RenderTargetHandle /*rt_handle*/) const override { return false; }
		void copy_target(RenderTargetHandle source_handle, RenderTargetHandle destination_handle) override;
		void remove_target(RenderTargetHandle rt_handle) override;

		void set_target(RenderTargetHandle rt_handle, DepthStencilHandle ds_handle) override;
		void clear_target(RenderTargetHandle rt_handle, const Vadon::Render::RGBAColor& clear_color) override;
		void clear_depth_stencil(DepthStencilHandle ds_handle, const DepthStencilClear& clear) override;
		
		bool is_depth_stencil_valid(DepthStencilHandle /*ds_handle*/) const override { return false; }
		void remove_depth_stencil(DepthStencilHandle ds_handle) override;

		void apply_viewport(const Viewport& viewport) override;

		RenderTargetHandle get_window_target(WindowHandle window_handle) const override;
	private:
		RenderTargetSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		GraphicsAPI& m_graphics_api;

		friend GraphicsAPI;
	};
}
#endif