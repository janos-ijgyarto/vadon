#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_RENDERTARGET_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_RENDERTARGET_HPP
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
namespace Vadon::Private::Render::DirectX
{
	using Viewport = Vadon::Render::Viewport;
	
	using WindowMode = Vadon::Render::WindowMode;

	using PlatformWindowHandle = Vadon::Render::PlatformWindowHandle;

	using WindowInfo = Vadon::Render::WindowInfo;

	using WindowHandle = Vadon::Render::WindowHandle;

	using RenderTargetInfo = Vadon::Render::RenderTargetInfo;

	using RenderTargetHandle = Vadon::Render::RenderTargetHandle;

	using DepthStencilViewInfo = Vadon::Render::DepthStencilViewInfo;

	using DepthStencilClear = Vadon::Render::DepthStencilClear;

	using DepthStencilHandle = Vadon::Render::DepthStencilHandle;

	using DXGISwapChain = ComPtr<IDXGISwapChain>;
	using D3DRenderTargetView = ComPtr<ID3D11RenderTargetView>;
	using D3DDepthStencilView = ComPtr<ID3D11DepthStencilView>;

	struct Window
	{
		WindowInfo info;
		HWND hwnd = nullptr;

		// FIXME: which exact IDXGI object should be used?
		DXGISwapChain swap_chain;
		RenderTargetHandle back_buffer;
	};

	struct RenderTarget
	{
		RenderTargetInfo info;
		D3DRenderTargetView d3d_rt_view;
	};

	struct DepthStencil
	{
		DepthStencilViewInfo info;
		D3DDepthStencilView d3d_ds_view;
	};
}
#endif