#ifndef VADONAPP_PLATFORM_PLATFORMINTERFACE_HPP
#define VADONAPP_PLATFORM_PLATFORMINTERFACE_HPP
#include <VadonApp/Platform/Module.hpp>

#include <VadonApp/Platform/Event/Event.hpp>
#include <VadonApp/Platform/Window/Window.hpp>

#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
namespace VadonApp::Platform
{
	// FIXME: more fitting name?
	// The features are based on metadata required by ImGui (might not be relevant for others?)
	enum class FeatureFlags
	{
		NONE = 0,
		MOUSE_GLOBAL_STATE = 1 << 0
	};

	class PlatformInterface : public PlatformSystem<PlatformInterface>
	{
	public:
		virtual ~PlatformInterface() {}

		// FIXME: might be better to register callbacks, cache in system, etc?
		virtual PlatformEventList read_events() = 0;

		// TODO: allow creating multiple windows!
		virtual RenderWindowInfo get_window_info() const = 0;
		virtual WindowHandle get_window_handle() const = 0;

		virtual void move_window(const Vadon::Utilities::Vector2i& position) = 0;
		virtual void resize_window(const Vadon::Utilities::Vector2i& size) = 0;

		virtual bool is_window_focused() const = 0;

		virtual FeatureFlags get_feature_flags() const = 0;
		virtual uint64_t get_performance_frequency() const = 0;
		virtual uint64_t get_performance_counter() const = 0;

		virtual void show_cursor(bool show) = 0;
		virtual void set_cursor(Cursor cursor) = 0;

		virtual void capture_mouse(bool capture) = 0;
		virtual void warp_mouse(const Vadon::Utilities::Vector2i& mouse_position) = 0;
		virtual Vadon::Utilities::Vector2i get_mouse_position() const = 0;

		virtual void set_clipboard_text(const char* text) = 0;
		virtual const char* get_clipboard_text() = 0;
	protected:
		PlatformInterface(Core::Application& application)
			: System(application)
		{}
	};
}

namespace Vadon::Utilities
{
	template <>
	struct EnableEnumBitwiseOperators<VadonApp::Platform::FeatureFlags> : public std::true_type
	{};
}
#endif