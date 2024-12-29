#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_RENDERTARGETSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_RENDERTARGETSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Resource.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/RenderTarget/RenderTarget.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Texture/Texture.hpp>

namespace Vadon::Private::Render::DirectX
{
	class GraphicsAPI;

	class RenderTargetSystem : public Vadon::Render::RenderTargetSystem
	{
	public:
		WindowHandle add_window(const WindowInfo& window_info) override;
		bool is_window_valid(WindowHandle window_handle) const override { return m_window_pool.is_handle_valid(window_handle); }
		WindowInfo get_window_info(WindowHandle window_handle) const override;
		void update_window(WindowHandle window_handle) override;
		void remove_window(WindowHandle window_handle) override;

		void resize_window(WindowHandle window_handle, const Vadon::Utilities::Vector2i& window_size) override;
		void set_window_mode(WindowHandle window_handle, WindowMode mode) override;

		RenderTargetHandle add_target(const RenderTargetInfo& rt_info, TextureHandle texture_handle) override;
		bool is_render_target_valid(RenderTargetHandle rt_handle) const override { return m_rt_pool.is_handle_valid(rt_handle); }
		void copy_target(RenderTargetHandle source_handle, RenderTargetHandle destination_handle) override;
		void remove_target(RenderTargetHandle rt_handle) override;

		void set_target(RenderTargetHandle rt_handle, DepthStencilHandle ds_handle) override;
		void clear_target(RenderTargetHandle rt_handle, const Vadon::Render::RGBAColor& clear_color) override;
		void clear_depth_stencil(DepthStencilHandle ds_handle, const DepthStencilClear& clear) override;

		bool is_depth_stencil_valid(DepthStencilHandle ds_handle) const override { return m_ds_pool.is_handle_valid(ds_handle); }
		void remove_depth_stencil(DepthStencilHandle ds_handle) override;

		void apply_viewport(const Viewport& viewport) override;

		RenderTargetHandle get_window_target(WindowHandle window_handle) const override;

		DepthStencilHandle create_depth_stencil_view(const D3DResource& resource, const DepthStencilViewInfo& ds_view_info);
	private:
		RenderTargetSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void update();
		void shutdown();

		using RenderTargetPool = Vadon::Utilities::ObjectPool<Vadon::Render::RenderTarget, RenderTarget>;
		using DepthStencilPool = Vadon::Utilities::ObjectPool<Vadon::Render::DepthStencil, DepthStencil>;
		using WindowPool = Vadon::Utilities::ObjectPool<Vadon::Render::Window, Window>;

		bool create_back_buffer_view(DXGISwapChain& swap_chain, D3DRenderTargetView& back_buffer_view);
		bool internal_create_rt_view(D3DRenderTargetView& rt_view, ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC* description);

		GraphicsAPI& m_graphics_api;

		RenderTargetPool m_rt_pool;
		DepthStencilPool m_ds_pool;
		WindowPool m_window_pool;

		friend GraphicsAPI;
	};
}
#endif