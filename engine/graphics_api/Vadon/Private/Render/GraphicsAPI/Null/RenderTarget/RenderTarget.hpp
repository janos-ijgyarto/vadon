#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_RENDERTARGET_RENDERTARGET_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_RENDERTARGET_RENDERTARGET_HPP
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>
namespace Vadon::Private::Render::Null
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
}
#endif