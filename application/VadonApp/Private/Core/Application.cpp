#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/Core/Application.hpp>

#include <Vadon/Core/Environment.hpp>

namespace VadonApp::Private::Core
{
    Application::Application()
        : m_engine(Vadon::Core::create_engine_core())
        , m_ui_system(*this)
    {
        // Redirect engine logs to console
        // FIXME: make sure we only set this once?
        Vadon::Core::EngineEnvironment::set_logger(&m_ui_system.get_console());
    }

    Application::~Application() = default;

    bool Application::initialize(const VadonApp::Core::Configuration& config)
    {
        log_message("Initializing Vadon Application.\n");

        m_config = config;

        if (m_engine->initialize(config.engine_config) == false)
        {
            log_error("Failed to initialize engine!\n");
            return false;
        }

        // FIXME: create platform interface based on command line and other configs
        m_platform_interface = Platform::PlatformInterface::get_interface(*this);

        if (m_platform_interface->initialize() == false)
        {
            log_error("Failed to initialize platform interface!\n");
            return false;
        }

        if (m_ui_system.initialize() == false)
        {
            log_error("Failed to initialize UI!\n");
            return false;
        }

        return true;
    }

    void Application::update()
    {
        // TODO!!!
    }

    void Application::shutdown()
    {
        m_engine->shutdown();
        if (m_platform_interface)
        {
            m_platform_interface->shutdown();
        }
        m_ui_system.shutdown();

        log_message("Vadon Application shut down successfully!\n");
    }
}