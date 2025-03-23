#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_RENDERTARGET_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_RENDERTARGET_HPP
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/Window.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/Resource.hpp>
namespace Vadon::Private::Render::DirectX
{
	using Viewport = Vadon::Render::Viewport;

	using RenderTargetViewType = Vadon::Render::RenderTargetViewType;

	using RenderTargetViewTypeInfo = Vadon::Render::RenderTargetViewTypeInfo;

	using RenderTargetViewInfo = Vadon::Render::RenderTargetViewInfo;

	using RTVHandle = Vadon::Render::RTVHandle;

	using D3DRenderTargetView = ComPtr<ID3D11RenderTargetView>;

	struct RenderTargetView
	{
		ResourceHandle resource;
		RenderTargetViewInfo info;
		D3DRenderTargetView d3d_rt_view;
		Vadon::Render::WindowHandle window; // NOTE: tracking in case this is the RT for a window back buffer
	};
}
#endif