#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_RENDERTARGETSYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_RENDERTARGETSYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/RenderTarget/RenderTarget.hpp>

namespace Vadon::Private::Render::DirectX
{
	class GraphicsAPI;

	class RenderTargetSystem : public Vadon::Render::RenderTargetSystem
	{
	public:
		WindowHandle add_window(const WindowInfo& window_info) override;
		WindowInfo get_window_info(WindowHandle window_handle) override;
		void update_window(WindowHandle window_handle) override;
		void remove_window(WindowHandle window_handle) override;

		void resize_window(WindowHandle window_handle, const Vadon::Utilities::Vector2i& window_size) override;
		void set_window_mode(WindowHandle window_handle, WindowMode mode) override;

		RenderTargetHandle add_target(const RenderTargetInfo& rt_info) override;
		void copy_target(RenderTargetHandle source_handle, RenderTargetHandle destination_handle) override;
		void remove_target(RenderTargetHandle rt_handle) override;

		void set_target(RenderTargetHandle rt_handle) override;
		void clear_target(RenderTargetHandle rt_handle, const Vadon::Render::RGBAColor& clear_color) override;

		void apply_viewport(const Viewport& viewport) override;

		RenderTargetHandle get_window_target(WindowHandle window_handle) override;
	private:
		RenderTargetSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void update();
		void shutdown();

		using RenderTargetPool = Vadon::Utilities::ObjectPool<Vadon::Render::RenderTarget, RenderTarget>;
		using WindowPool = Vadon::Utilities::ObjectPool<Vadon::Render::Window, Window>;

		bool update_back_buffer_view(DXGISwapChain& swap_chain, D3DRenderTargetView& back_buffer_view);

		GraphicsAPI& m_graphics_api;

		RenderTargetPool m_rt_pool;
		WindowPool m_window_pool;

		friend GraphicsAPI;
	};
}
#endif