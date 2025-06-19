#include <VadonDemo/Core/Editor.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Model/Model.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>
#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/Platform/Input/InputSystem.hpp>

#include <Vadon/Core/Core.hpp>
#include <Vadon/Core/CoreConfiguration.hpp>

#include <thread>

namespace VadonDemo::Core
{
    Editor::Editor(Vadon::Core::EngineEnvironment& environment)
        : m_engine_core(Vadon::Core::create_engine_core())
        , m_common_editor(environment, *m_engine_core)
        , m_platform(*this)
        , m_render(*this)
        , m_ui(*this)
        , m_view(*this)
        , m_running(true)
        , m_delta_time(0.0f)
    {
        VadonDemo::Core::Core::init_environment(environment);
    }

    Editor::~Editor() = default;

    int Editor::execute(int argc, char* argv[])
    {
        if (initialize(argc, argv) == false)
        {
            return -1;
        }

        using Clock = std::chrono::steady_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Duration = std::chrono::duration<float>;

        TimePoint last_frame_time = Clock::now();

        while (m_running == true)
        {
            TimePoint current_time = Clock::now();
            const float delta_time = std::chrono::duration_cast<Duration>(current_time - last_frame_time).count();

            // Limit framerate (input response is better this way)
            if (delta_time < (1.0f / 60.0f))
            {
                std::this_thread::yield();
                continue;
            }

            m_delta_time = delta_time;
            last_frame_time = current_time;

            begin_frame();

            // Check project state
            VadonEditor::Core::ProjectManager& project_manager = m_common_editor.get_system<VadonEditor::Core::ProjectManager>();
            switch (project_manager.get_state())
            {
            case VadonEditor::Core::ProjectManager::State::LAUNCHER:
            {
                m_ui.update();
                m_render.update();
            }
            break;
            case VadonEditor::Core::ProjectManager::State::PROJECT_OPEN:
            {
                if (project_manager.load_project_data() == false)
                {
                    VADON_ERROR("Failed to load project data!");
                    return -1;
                }

                if (project_loaded() == false)
                {
                    VADON_ERROR("Failed to initialize demo subsystems!");
                    return -1;
                }
            }
            break;
            case VadonEditor::Core::ProjectManager::State::PROJECT_LOADED:
                update_subsystems();
                break;
            case VadonEditor::Core::ProjectManager::State::PROJECT_CLOSED:
                m_running = false;
                break;
            }
        }

        shutdown();
        return 0;
    }

    void Editor::shutdown()
    {
        // TODO: shut down demo subsystems (clean up resources, etc.)
        m_common_editor.shutdown();
        m_engine_core->shutdown();
    }

    void Editor::begin_frame()
    {
        VadonApp::Core::Application& application = m_common_editor.get_engine_app();
        VadonApp::Platform::PlatformInterface& platform_interface = application.get_system<VadonApp::Platform::PlatformInterface>();

        platform_interface.new_frame();

        // FIXME: make this more concise using std::visit?
        for (const VadonApp::Platform::PlatformEvent& current_event : platform_interface.poll_events())
        {
            const VadonApp::Platform::PlatformEventType current_event_type = Vadon::Utilities::to_enum<VadonApp::Platform::PlatformEventType>(static_cast<int32_t>(current_event.index()));
            switch (current_event_type)
            {
            case VadonApp::Platform::PlatformEventType::QUIT:
            {
                // Platform is trying to quit, so we request stop
                m_running = false;
            }
            break;
            }
        }
    }

    void Editor::update_subsystems()
    {
        m_common_editor.get_engine_app().get_system<VadonApp::Platform::InputSystem>().update();

        m_view.update();
        m_ui.update();
        m_render.update();
    }

    const VadonEditor::Model::Scene* Editor::find_entity_scene(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_common_editor.get_system<VadonEditor::Model::ModelSystem>();
        return editor_model.get_scene_system().find_entity_scene(entity);
    }

    bool Editor::initialize(int argc, char* argv[])
    {
        // First parse the command line
        m_common_editor.get_engine_app().parse_command_line(argc, argv);

        // TODO: use command line to set up configs!
        Vadon::Core::CoreConfiguration engine_config;
        if (m_engine_core->initialize(engine_config) == false)
        {
            // TODO: error!
            return false;
        }

        // Initialize the editor
        VadonEditor::Core::Configuration editor_config;
        if (m_common_editor.initialize(editor_config) == false)
        {
            return false;
        }

        // FIXME: this needs to be done here to ensure that the parent types are already available
        // Should refactor to instead have "auto-registering" (via macros on the declarations and static vars)
        // that enqueues type registry metadata, and then use one explicit call to process all of it.
        VadonDemo::Core::Core::register_types();

        VadonEditor::Core::ProjectManager& editor_project_manager = m_common_editor.get_system<VadonEditor::Core::ProjectManager>();

        editor_project_manager.set_project_custom_properties(VadonDemo::Core::GlobalConfiguration::get_default_properties());

        editor_project_manager.register_project_properties_callback(
            [this](const VadonEditor::Core::Project& project)
            {
                m_core->override_global_config(project.info);
                m_render.update_editor_layer();
            }
        );

        if (m_render.initialize() == false)
        {
            return false;
        }

        return true;
    }

    bool Editor::project_loaded()
    {
        // Retrieve config data to make it available to subsystems
        const VadonEditor::Core::Project& active_project = m_common_editor.get_system<VadonEditor::Core::ProjectManager>().get_active_project();

        m_core = std::make_unique<Core>(*m_engine_core);
        if (m_core->initialize(active_project.info) == false)
        {
            return false;
        }

        if (m_platform.initialize() == false)
        {
            return false;
        }

        if (m_render.project_loaded() == false)
        {
            return false;
        }

        if (m_view.initialize() == false)
        {
            return false;
        }

        if (m_ui.initialize() == false)
        {
            return false;
        }

        return true;
    }
}