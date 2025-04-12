#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_RENDERTARGET_RENDERTARGET_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_RENDERTARGET_RENDERTARGET_HPP
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>
namespace Vadon::Private::Render::Null
{
	using Viewport = Vadon::Render::Viewport;

	using PlatformWindowHandle = Vadon::Render::PlatformWindowHandle;

	using WindowInfo = Vadon::Render::WindowInfo;

	using WindowHandle = Vadon::Render::WindowHandle;

	using WindowUpdateInfo = Vadon::Render::WindowUpdateInfo;

	using WindowUpdateResult = Vadon::Render::WindowUpdateResult;
}
#endif