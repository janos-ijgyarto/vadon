#ifndef VADON_RENDER_GRAPHICSAPI_RENDERTARGET_RENDERTARGET_HPP
#define VADON_RENDER_GRAPHICSAPI_RENDERTARGET_RENDERTARGET_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/Math/Rectangle.hpp>
namespace Vadon::Render
{
	// Maps vertex positions (in clip space) into render target positions (i.e decides where the result of rendering ends up in the actual target)
	struct Viewport
	{
		Utilities::Rectangle dimensions = { { 0, 0 }, { 0, 0 } };
		Utilities::Vector2 depth_min_max = Utilities::Vector2(0.0f, 1.0f);
	};

	enum class WindowMode
	{
		WINDOWED,
		FULLSCREEN,
		BORDERLESS
	};

	using PlatformWindowHandle = void*; // Platform-dependent window handle

	struct WindowInfo
	{
		Utilities::Vector2u dimensions;
		PlatformWindowHandle platform_handle = nullptr;
		// TODO: other info
		// - vsync, refresh rate
		// - buffer count
		// - buffer format (and usage?)
		// - MSAA
		// - Scan line ordering
		// - Scaling
		// - Swap effect (e.g discard)
		// - Flags (?)
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Window, WindowHandle);

	struct RenderTargetInfo
	{
		Utilities::Vector2u dimensions;
		// TODO: other parameters!!!
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(RenderTarget, RenderTargetHandle);

	// TODO: depth-stencil parameters!
	struct DepthStencilViewInfo
	{

	};

	enum class DepthStencilClearFlags
	{
		NONE = 0,
		DEPTH = 1 << 0,
		STENCIL = 1 << 1
	};

	struct DepthStencilClear
	{
		DepthStencilClearFlags clear_flags = DepthStencilClearFlags::NONE;
		float depth = 0.0f;
		uint8_t stencil = 0;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(DepthStencil, DepthStencilHandle);
}
#endif