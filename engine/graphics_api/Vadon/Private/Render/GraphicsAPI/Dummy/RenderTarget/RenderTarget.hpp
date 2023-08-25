#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_RENDERTARGET_RENDERTARGET_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_RENDERTARGET_RENDERTARGET_HPP
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>
namespace Vadon::Private::Render::Dummy
{
	using Viewport = Vadon::Render::Viewport;

	using WindowMode = Vadon::Render::WindowMode;

	using PlatformWindowHandle = Vadon::Render::PlatformWindowHandle;

	using WindowInfo = Vadon::Render::WindowInfo;

	using WindowHandle = Vadon::Render::WindowHandle;

	using RenderTargetInfo = Vadon::Render::RenderTargetInfo;

	using RenderTargetHandle = Vadon::Render::RenderTargetHandle;
}
#endif