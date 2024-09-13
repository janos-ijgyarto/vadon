#ifndef VADONAPP_PRIVATE_PLATFORM_PLATFORMINTERFACE_HPP
#define VADONAPP_PRIVATE_PLATFORM_PLATFORMINTERFACE_HPP
#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/Private/Platform/Input/InputSystem.hpp>
namespace VadonApp::Private::Platform
{
	class PlatformInterface : public VadonApp::Platform::PlatformInterface
	{
	public:
		using Implementation = std::unique_ptr<PlatformInterface>;

		static Implementation get_interface(VadonApp::Core::Application& application);
		static Implementation get_dummy_interface(VadonApp::Core::Application& application);

		bool initialize();
		void shutdown();
	protected:
		PlatformInterface(VadonApp::Core::Application& application);

		virtual bool initialize_internal() = 0;
		virtual void shutdown_internal() = 0;

		InputSystem m_input_system;
	};
}
#endif