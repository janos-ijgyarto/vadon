#ifndef VADON_RENDER_GRAPHICSAPI_RENDERTARGET_WINDOW_HPP
#define VADON_RENDER_GRAPHICSAPI_RENDERTARGET_WINDOW_HPP
#include <Vadon/Render/GraphicsAPI/Defines.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>
namespace Vadon::Render
{
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
}
#endif