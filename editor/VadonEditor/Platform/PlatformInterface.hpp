#ifndef VADONEDITOR_PLATFORM_PLATFORMINTERFACE_HPP
#define VADONEDITOR_PLATFORM_PLATFORMINTERFACE_HPP
#include <VadonEditor/Platform/Module.hpp>
#include <VadonEditor/Platform/Event.hpp>
namespace VadonEditor::Platform
{
	using PlatformEventCallback = std::function<void(const VadonApp::Platform::PlatformEventList&)>;

	class PlatformInterface : public PlatformSystem<PlatformInterface>
	{
	public:
		~PlatformInterface();

		void register_event_callback(const PlatformEventCallback& callback);
	private:
		PlatformInterface(Core::Editor& editor);

		bool initialize();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Editor;
	};
}
#endif