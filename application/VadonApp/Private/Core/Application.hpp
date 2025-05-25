#ifndef VADONAPP_PRIVATE_CORE_APPLICATION_HPP
#define VADONAPP_PRIVATE_CORE_APPLICATION_HPP
#include <VadonApp/Core/Application.hpp>

#include <VadonApp/Private/Platform/PlatformInterface.hpp>
#include <VadonApp/Private/UI/UISystem.hpp>

namespace VadonApp::Private::Core
{
	class Application final : public VadonApp::Core::Application
	{
	public:
		Application(Vadon::Core::EngineCoreInterface& engine_core);
		~Application();

		bool initialize(const VadonApp::Core::Configuration& config = VadonApp::Core::Configuration()) override;
		void shutdown() override;

		const VadonApp::Core::Configuration& get_config() const override { return m_config; }
	private:
		VadonApp::Core::Configuration m_config;

		Platform::PlatformInterface::Implementation m_platform_interface;
		UI::UISystem m_ui_system;
	};
}
#endif