#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_RENDERTARGETSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_RENDERTARGETSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/RenderTarget/DSV.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/RenderTarget/RenderTarget.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/RenderTarget/Window.hpp>

namespace Vadon::Private::Render::DirectX
{
	class GraphicsAPI;

	class RenderTargetSystem : public Vadon::Render::RenderTargetSystem
	{
	public:
		WindowHandle create_window(const WindowInfo& window_info) override;
		bool is_window_valid(WindowHandle window_handle) const override { return m_window_pool.is_handle_valid(window_handle); }
		WindowInfo get_window_info(WindowHandle window_handle) const override;
		WindowUpdateResult update_window(const WindowUpdateInfo& info) override;
		void remove_window(WindowHandle window_handle) override;

		void resize_window(WindowHandle window_handle, const Vadon::Math::Vector2i& window_size) override;

		RTVHandle create_render_target_view(ResourceHandle resource_handle, const RenderTargetViewInfo& rtv_info) override;
		bool is_rtv_valid(RTVHandle rtv_handle) const override { return m_rtv_pool.is_handle_valid(rtv_handle); }
		void remove_render_target_view(RTVHandle rtv_handle) override;

		DSVHandle create_depth_stencil_view(ResourceHandle resource_handle, const DepthStencilViewInfo& dsv_info) override;
		bool is_dsv_valid(DSVHandle dsv_handle) const override { return m_dsv_pool.is_handle_valid(dsv_handle); }
		void remove_depth_stencil_view(DSVHandle dsv_handle) override;

		void set_target(RTVHandle rtv_handle, DSVHandle dsv_handle) override;
		void clear_target(RTVHandle rtv_handle, const Vadon::Math::Vector4& clear_color) override;
		void clear_depth_stencil(DSVHandle dsv_handle, const DepthStencilClear& clear) override;

		void apply_viewport(const Viewport& viewport) override;

		RTVHandle get_window_back_buffer_view(WindowHandle window_handle) override;
	private:
		RenderTargetSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void update();
		void shutdown();

		using RenderTargetViewPool = Vadon::Utilities::ObjectPool<Vadon::Render::RenderTargetView, RenderTargetView>;
		using DepthStencilViewPool = Vadon::Utilities::ObjectPool<Vadon::Render::DepthStencilView, DepthStencilView>;
		using WindowPool = Vadon::Utilities::ObjectPool<Vadon::Render::Window, Window>;

		bool create_back_buffer_view(DXGISwapChain& swap_chain, D3DRenderTargetView& back_buffer_view);
		bool internal_create_rtv(D3DRenderTargetView& d3d_rtv, ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC* description);

		GraphicsAPI& m_graphics_api;

		RenderTargetViewPool m_rtv_pool;
		DepthStencilViewPool m_dsv_pool;
		WindowPool m_window_pool;

		friend GraphicsAPI;
	};
}
#endif