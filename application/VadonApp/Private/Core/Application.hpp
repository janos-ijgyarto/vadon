#ifndef VADONAPP_PRIVATE_CORE_APPLICATION_HPP
#define VADONAPP_PRIVATE_CORE_APPLICATION_HPP
#include <VadonApp/Core/Application.hpp>

#include <Vadon/Core/Core.hpp>

#include <VadonApp/Private/Platform/PlatformInterface.hpp>
#include <VadonApp/Private/UI/UISystem.hpp>

namespace VadonApp::Private::Core
{
	class Application final : public VadonApp::Core::Application
	{
	public:
		Application();
		~Application();

		bool initialize(const VadonApp::Core::Configuration& config = VadonApp::Core::Configuration()) override;
		void update() override;
		void shutdown() override;

		Vadon::Core::EngineCoreInterface& get_engine_core() override { return *m_engine; }

		const VadonApp::Core::Configuration& get_config() const override { return m_config; }
	private:
		Vadon::Core::EngineCoreImpl m_engine;
		VadonApp::Core::Configuration m_config;

		Platform::PlatformInterface::Implementation m_platform_interface;
		UI::UISystem m_ui_system;
	};
}
#endif