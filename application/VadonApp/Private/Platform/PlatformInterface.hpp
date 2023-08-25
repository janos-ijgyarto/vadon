#ifndef VADONAPP_PRIVATE_PLATFORM_PLATFORMINTERFACE_HPP
#define VADONAPP_PRIVATE_PLATFORM_PLATFORMINTERFACE_HPP
#include <VadonApp/Platform/PlatformInterface.hpp>
namespace VadonApp::Private::Platform
{
	class PlatformInterface : public VadonApp::Platform::PlatformInterface
	{
	public:
		using Implementation = std::unique_ptr<PlatformInterface>;

		virtual bool initialize() = 0;
		virtual void shutdown() = 0;

		static Implementation get_interface(VadonApp::Core::Application& application);
		static Implementation get_dummy_interface(VadonApp::Core::Application& application);
	protected:
		PlatformInterface(VadonApp::Core::Application& application) : VadonApp::Platform::PlatformInterface(application) {}
	};
}
#endif