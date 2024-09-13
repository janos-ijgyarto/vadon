#ifndef VADONDEMO_PLATFORM_PLATFORMINTERFACE_HPP
#define VADONDEMO_PLATFORM_PLATFORMINTERFACE_HPP
#include <Vadon/Utilities/Math/Vector.hpp>
#include <VadonApp/Platform/Event/Event.hpp>
#include <VadonApp/Platform/Input/Input.hpp>
#include <memory>
namespace VadonDemo::Core
{
	class GameCore;
}
namespace VadonDemo::Platform
{
	using PlatformEventCallback = std::function<void(const VadonApp::Platform::PlatformEventList&)>;

	class PlatformInterface
	{
	public:
		struct InputValues
		{
			bool camera_up = false;
			bool camera_down = false;
			bool camera_left = false;
			bool camera_right = false;
			float camera_zoom = 0.0f;
		};

		~PlatformInterface();

		void register_event_callback(const PlatformEventCallback& callback);

		void move_window(Vadon::Utilities::Vector2i position);
		void resize_window(Vadon::Utilities::Vector2i size);

		InputValues get_input_values() const;
	private:
		PlatformInterface(Core::GameCore& game_core);

		bool initialize();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::GameCore;
	};
}
#endif