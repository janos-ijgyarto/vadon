#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/Core/Application.hpp>

#include <VadonApp/UI/Console.hpp>

#include <Vadon/Core/CoreInterface.hpp>
#include <Vadon/Core/Logger.hpp>

namespace VadonApp::Private::Core
{
    Application::Application()
        : m_engine(Vadon::Core::create_engine_core())
        , m_ui_system(*this)
    {
        // Redirect engine logs to console
        m_engine->set_logger(&m_ui_system.get_console());
    }

    Application::~Application() = default;

    bool Application::initialize(const VadonApp::Core::Configuration& config)
    {
        Vadon::Core::Logger& logger = m_engine->get_logger();
        logger.log("Initializing Vadon Application.\n");

        m_config = config;

        if (m_engine->initialize(config.engine_config) == false)
        {
            logger.error("Failed to initialize engine!\n");
            return false;
        }

        // FIXME: create platform interface based on command line and other configs
        m_platform_interface = Platform::PlatformInterface::get_interface(*this);

        if (m_platform_interface->initialize() == false)
        {
            logger.error("Failed to initialize platform interface!\n");
            return false;
        }

        if (m_ui_system.initialize() == false)
        {
            logger.error("Failed to initialize UI!\n");
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

        Vadon::Core::Logger& logger = m_engine->get_logger();
        logger.log("Vadon Application shut down successfully!\n");
    }
}