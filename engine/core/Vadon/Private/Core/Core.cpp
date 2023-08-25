#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Core/Core.hpp>

#include <Vadon/Core/Core.hpp>

#include <iostream>
#include <syncstream>

namespace Vadon::Core
{
    EngineCoreImpl create_engine_core()
    {
        return std::make_unique<Vadon::Private::Core::EngineCore>();
    }
}

namespace Vadon::Private::Core
{
    class EngineCore::DefaultLogger : public Vadon::Core::Logger
    {
    public:
        void log(std::string_view message)
        {
            std::osyncstream cout_sync(std::cout);
            cout_sync << message;
        }

        void warning(std::string_view message)
        {
            // TODO: timestamp?
            std::osyncstream cout_sync(std::cout);
            cout_sync << "WARNING: " << message;
        }

        void error(std::string_view message)
        {
            // TODO: timestamp?
            std::osyncstream cerr_sync(std::cerr);
            cerr_sync << "ERROR: " << message;
        }
    };

    EngineCore::EngineCore()
        : m_task_system(*this)
        , m_render_system(*this)
        , m_default_logger(std::make_unique<DefaultLogger>())
    {

    }

    EngineCore::~EngineCore() = default;

    bool EngineCore::initialize(const Vadon::Core::Configuration& config)
    {
        m_logger->log("Vadon engine core initializing.\n");
        constexpr const char* c_failure_message = "Vadon engine core initialization failed!\n";

        m_config = config;

        if (m_task_system.initialize() == false)
        {
            m_logger->log(c_failure_message);
            return false;
        }

        if (m_render_system.initialize() == false)
        {
            m_logger->log(c_failure_message);
            return false;
        }

        m_graphics_api = Render::GraphicsAPIBase::get_graphics_api(*this);
        if (m_graphics_api->initialize() == false)
        {
            m_logger->log(c_failure_message);
            return false;
        }

        m_logger->log("Vadon initialized successfully.\n");
        return true;
    }

    void EngineCore::update()
    {
        m_render_system.update();
        m_graphics_api->update();
    }

    void EngineCore::shutdown()
    {
        m_logger->log("Vadon shutting down.\n");

        m_task_system.shutdown();
        m_render_system.shutdown();
        m_graphics_api->shutdown();

        m_logger->log("Vadon shut down successfully.\n");
    }

    void EngineCore::set_logger(Vadon::Core::Logger* logger)
    {
        if (logger)
        {
            m_logger = logger;
        }
        else
        {
            m_logger = m_default_logger.get();
        }
    }
}