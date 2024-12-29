#ifndef VADON_RENDER_GRAPHICSAPI_RENDERTARGET_RENDERTARGET_HPP
#define VADON_RENDER_GRAPHICSAPI_RENDERTARGET_RENDERTARGET_HPP
#include <Vadon/Render/GraphicsAPI/Defines.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/Math/Rectangle.hpp>
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
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
		PlatformWindowHandle platform_handle = nullptr;
		Utilities::Vector2u size = { 0,0 };
		GraphicsAPIDataFormat format = GraphicsAPIDataFormat::UNKNOWN;
		uint32_t buffer_count = 2;
		// TODO: other parameters?
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Window, WindowHandle);

	// TODO: render target parameters!
	struct RenderTargetInfo
	{
		Utilities::Vector2u dimensions;
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
namespace Vadon::Utilities
{
	template<>
	struct EnableEnumBitwiseOperators<Vadon::Render::DepthStencilClearFlags> : public std::true_type
	{

	};
}
#endif