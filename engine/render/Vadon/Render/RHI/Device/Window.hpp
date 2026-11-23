#ifndef VADON_RENDER_RHI_DEVICE_WINDOW_HPP
#define VADON_RENDER_RHI_DEVICE_WINDOW_HPP
#include <Vadon/Render/RHI/Device/Queue.hpp>
namespace Vadon::Render::RHI
{
	enum class WindowFormat
	{
		BT709_G10_16BIT,
		DEFAULT = BT709_G10_16BIT
	};

	struct PlatformWindowHandle
	{
		void* window_handle = nullptr;
	};

	struct WindowDescription
	{
		PlatformWindowHandle platform_handle = {};
		QueueHandle queue;
		uint16_t width = 0;
		uint16_t height = 0;
		WindowFormat format = WindowFormat::DEFAULT;
		bool vsync = false;
		bool allow_tearing = false;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Window, WindowHandle);
}
#endif