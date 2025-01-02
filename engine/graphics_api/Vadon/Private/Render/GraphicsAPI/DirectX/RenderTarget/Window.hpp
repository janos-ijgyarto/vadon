#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_WINDOW_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_WINDOW_HPP
#include <Vadon/Render/GraphicsAPI/RenderTarget/Window.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
namespace Vadon::Private::Render::DirectX
{
	using PlatformWindowHandle = Vadon::Render::PlatformWindowHandle;

	using WindowInfo = Vadon::Render::WindowInfo;

	using WindowHandle = Vadon::Render::WindowHandle;

	using DXGISwapChain = ComPtr<IDXGISwapChain>;

	struct Window
	{
		WindowInfo info;
		HWND hwnd = nullptr;

		// FIXME: which exact IDXGI object should be used?
		DXGISwapChain swap_chain;
		RTVHandle back_buffer_rtv;
	};
}
#endif