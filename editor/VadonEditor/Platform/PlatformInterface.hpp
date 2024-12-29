#ifndef VADONEDITOR_PLATFORM_PLATFORMINTERFACE_HPP
#define VADONEDITOR_PLATFORM_PLATFORMINTERFACE_HPP
#include <VadonEditor/Platform/Module.hpp>
#include <VadonApp/Platform/Window/Window.hpp>
namespace VadonEditor::Platform
{
	class PlatformInterface : public PlatformSystem<PlatformInterface>
	{
	public:
		~PlatformInterface();

		VADONEDITOR_API VadonApp::Platform::WindowHandle get_main_window() const;
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