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
		WindowHandle create_window(const WindowInfo& window_info) override;
		bool is_window_valid(WindowHandle /*window_handle*/) const override { return false; }
		WindowInfo get_window_info(WindowHandle window_handle) const override;
		WindowUpdateResult update_window(const WindowUpdateInfo& info) override;
		void remove_window(WindowHandle window_handle) override;

		void resize_window(WindowHandle window_handle, const Vadon::Math::Vector2i& window_size) override;

		Vadon::Render::RTVHandle create_render_target_view(Vadon::Render::ResourceHandle resource_handle, const Vadon::Render::RenderTargetViewInfo& rtv_info) override;
		bool is_rtv_valid(Vadon::Render::RTVHandle /*rtv_handle*/) const override { return false; }
		void remove_render_target_view(Vadon::Render::RTVHandle rtv_handle) override;

		Vadon::Render::DSVHandle create_depth_stencil_view(Vadon::Render::ResourceHandle resource_handle, const Vadon::Render::DepthStencilViewInfo& dsv_info) override;
		bool is_dsv_valid(Vadon::Render::DSVHandle /*dsv_handle*/) const override { return false; }
		void remove_depth_stencil_view(Vadon::Render::DSVHandle dsv_handle) override;

		void set_target(Vadon::Render::RTVHandle rtv_handle, Vadon::Render::DSVHandle dsv_handle) override;
		void clear_target(Vadon::Render::RTVHandle rtv_handle, const Vadon::Math::Vector4& clear_color) override;
		void clear_depth_stencil(Vadon::Render::DSVHandle dsv_handle, const Vadon::Render::DepthStencilClear& clear) override;

		void apply_viewport(const Viewport& viewport) override;

		Vadon::Render::RTVHandle get_window_back_buffer_view(WindowHandle window_handle) override;
	private:
		RenderTargetSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		GraphicsAPI& m_graphics_api;

		friend GraphicsAPI;
	};
}
#endif