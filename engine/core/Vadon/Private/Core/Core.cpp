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

    EngineCoreImpl create_engine_core()
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
        , m_scene_system(*this)
    {
    }

    EngineCore::~EngineCore() = default;

    bool EngineCore::initialize(const Vadon::Core::Configuration& config)
    {
        log_message("Vadon engine core initializing.\n");
        constexpr const char* c_failure_message = "Vadon engine core initialization failed!\n";

        m_config = config;

        if (m_file_system.initialize() == false)
        {
            log_message(c_failure_message);
            return false;
        }

        if (m_task_system.initialize() == false)
        {
            log_message(c_failure_message);
            return false;
        }

        m_graphics_api = Render::GraphicsAPIBase::get_graphics_api(*this);
        if (m_graphics_api->initialize() == false)
        {
            log_error(c_failure_message);
            return false;
        }

        if (m_render_system.initialize() == false)
        {
            log_error(c_failure_message);
            return false;
        }

        if (m_scene_system.initialize() == false)
        {
            log_error(c_failure_message);
            return false;
        }

        log_message("Vadon initialized successfully.\n");
        return true;
    }

    void EngineCore::update()
    {
        m_render_system.update();
        m_graphics_api->update();
    }

    void EngineCore::shutdown()
    {
        log_message("Vadon shutting down.\n");

        m_scene_system.shutdown();
        m_task_system.shutdown();
        m_render_system.shutdown();
        if (m_graphics_api != nullptr)
        {
            m_graphics_api->shutdown();
        }

        log_message("Vadon shut down successfully.\n");
    }
}