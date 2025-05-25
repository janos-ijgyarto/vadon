#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/Core/Application.hpp>

#include <Vadon/Core/Environment.hpp>

namespace VadonApp::Private::Core
{
    Application::Application(Vadon::Core::EngineCoreInterface& engine_core)
        : VadonApp::Core::Application(engine_core)
        , m_ui_system(*this)
    {
    }

    Application::~Application() = default;

    bool Application::initialize(const VadonApp::Core::Configuration& config)
    {
        Vadon::Core::Logger::log_message("Initializing Vadon Application.\n");

        m_config = config;

        // FIXME: create platform interface based on command line and other configs
        m_platform_interface = Platform::PlatformInterface::get_interface(*this);

        if (m_platform_interface->initialize() == false)
        {
            Vadon::Core::Logger::log_error("Failed to initialize platform interface!\n");
            return false;
        }

        if (m_ui_system.initialize() == false)
        {
            Vadon::Core::Logger::log_error("Failed to initialize UI!\n");
            return false;
        }

        return true;
    }

    void Application::shutdown()
    {
        if (m_platform_interface)
        {
            m_platform_interface->shutdown();
        }
        m_ui_system.shutdown();

        Vadon::Core::Logger::log_message("Vadon Application shut down successfully!\n");
    }
}