#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Core/Core.hpp>

#include <Vadon/Core/Core.hpp>
#include <Vadon/Core/Environment.hpp>

#include <Vadon/Private/Render/RenderSystem.hpp>

namespace Vadon::Core
{
    void init_engine_environment(EngineEnvironment& environment)
    {
        // Make sure all modules have the environment initialized with the same instance
        EngineEnvironment::initialize(environment);

        Vadon::Private::Render::RenderSystem::init_engine_environment(environment);
        Vadon::Private::Render::GraphicsAPIBase::init_engine_environment(environment);
    }

    EngineCorePtr create_engine_core()
    {
        return std::make_unique<Vadon::Private::Core::EngineCore>();
    }
}

namespace Vadon::Private::Core
{
    EngineCore::EngineCore()
        : m_file_system(*this)
        , m_task_system(*this) 
        , m_render_system(*this)
        , m_resource_system(*this)
        , m_scene_system(*this)
    {
    }

    EngineCore::~EngineCore() = default;

    bool EngineCore::initialize(const Vadon::Core::CoreConfiguration& config)
    {
        Vadon::Core::Logger::log_message("Vadon engine core initializing.\n");
        constexpr const char* c_failure_message = "Vadon engine core initialization failed!\n";

        m_config = config;

        if (m_file_system.initialize() == false)
        {
            Vadon::Core::Logger::log_message(c_failure_message);
            return false;
        }

        if (m_task_system.initialize() == false)
        {
            Vadon::Core::Logger::log_message(c_failure_message);
            return false;
        }

        // TODO: use configuration to select graphics backend
        m_graphics_api = Render::GraphicsAPIBase::get_graphics_api(*this);
        if (m_graphics_api->initialize() == false)
        {
            Vadon::Core::Logger::log_error(c_failure_message);
            return false;
        }

        if (m_render_system.initialize() == false)
        {
            Vadon::Core::Logger::log_error(c_failure_message);
            return false;
        }

        if (m_resource_system.initialize() == false)
        {
            Vadon::Core::Logger::log_error(c_failure_message);
            return false;
        }

        if (m_scene_system.initialize() == false)
        {
            Vadon::Core::Logger::log_error(c_failure_message);
            return false;
        }

        Vadon::Core::Logger::log_message("Vadon initialized successfully.\n");
        return true;
    }

    void EngineCore::shutdown()
    {
        Vadon::Core::Logger::log_message("Vadon shutting down.\n");

        m_scene_system.shutdown();
        m_resource_system.shutdown();
        m_task_system.shutdown();
        m_render_system.shutdown();
        if (m_graphics_api != nullptr)
        {
            m_graphics_api->shutdown();
        }

        Vadon::Core::Logger::log_message("Vadon shut down successfully.\n");
    }
}