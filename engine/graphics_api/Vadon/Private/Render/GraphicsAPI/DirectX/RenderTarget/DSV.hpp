#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_DSV_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RENDERTARGET_DSV_HPP
#include <Vadon/Render/GraphicsAPI/RenderTarget/DSV.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/Resource.hpp>
namespace Vadon::Private::Render::DirectX
{
	using DepthStencilViewType = Vadon::Render::DepthStencilViewType;
	using DepthStencilViewFlags = Vadon::Render::DepthStencilViewFlags;

	using DepthStencilViewInfo = Vadon::Render::DepthStencilViewInfo;

	using DepthStencilClear = Vadon::Render::DepthStencilClear;

	using DSVHandle = Vadon::Render::DSVHandle;

	using D3DDepthStencilView = ComPtr<ID3D11DepthStencilView>;

	struct DepthStencilView
	{
		ResourceHandle resource;
		DepthStencilViewInfo info;
		D3DDepthStencilView d3d_ds_view;
	};
}
#endif
