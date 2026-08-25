#include <VadonDemo/Core/Editor.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Model/Model.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>
#include <VadonEditor/Core/TypeInfo/MetadataRegistry.hpp>

#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Core/Core.hpp>
#include <Vadon/Core/CoreConfiguration.hpp>
#include <Vadon/Core/Environment.hpp>

#include <Vadon/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/System/CommandLine/Parser.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Plugin.hpp>
#include <Vadon/Foundation/Editor/Simulator/SimulatorInterface.hpp>

#include <thread>

namespace VadonDemo::Core
{
    EditorLogger::EditorLogger(::Vadon::Foundation::EditorSimulatorInterface& simulator_interface)
        : Logger(::Vadon::Foundation::EditorPluginMessageSource::SIMULATOR)
        , m_simulator_interface(simulator_interface)
    {
    }

    void EditorLogger::dispatch_message_data(const char* data, size_t size)
    {
        m_simulator_interface.dispatch_message_to_editor(data, size);
    }

    Editor::Editor(::Vadon::Foundation::EditorSimulatorInterface& simulator_interface)
        : ::Vadon::Foundation::EditorSimulatorPluginInterface(simulator_interface)
        , m_engine_core(Vadon::Core::create_engine_core())
        , m_common_editor(*m_engine_core)
        , m_logger(simulator_interface)
        , m_platform(*this)
        , m_render(*this)
        , m_ui(*this)
        , m_view(*this)
        , m_running(true)
        , m_delta_time(0.0f)
    {
        m_last_frame_time = Clock::now();

        Vadon::Core::EngineEnvironment::set_logger(&m_logger);
    }

    Editor::~Editor()
    {
        Vadon::Core::EngineEnvironment::set_logger(nullptr);
    }

    void Editor::init_environment(Vadon::Core::EngineEnvironment& environment)
    {
        VadonEditor::Core::Editor::init_environment(environment);
        VadonDemo::Core::Core::init_environment(environment);
    }

    void Editor::register_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& registry)
    {
        Vadon::Core::register_engine_types();
        Vadon::Core::register_engine_type_metadata(registry);

        // FIXME: currently we can register the types here, but ideally we should have a system that tracks
        // the dependencies in the type registry and ensures they are registered in the correct order
        VadonDemo::Core::Core::register_types(registry);
    }

    void Editor::update_subsystems()
    {
        m_view.update();
        m_ui.update();
        m_render.update();
    }

    bool Editor::initialize(const char* project_path)
    {
        // TODO: use command line to set up configs!
        Vadon::Core::CoreConfiguration engine_config;
        if (m_engine_core->initialize(engine_config) == false)
        {
            // TODO: error!
            return false;
        }

        // Initialize the editor
        if (m_common_editor.initialize() == false)
        {
            return false;
        }

        if (m_platform.initialize() == false)
        {
            return false;
        }

        if (m_render.initialize() == false)
        {
            return false;
        }

        // Add a callback for when the global config is modified
        VadonEditor::Model::ResourceSystem& editor_resource_system = m_common_editor.get_resource_system();
        editor_resource_system.register_event_callback(
            [this](const VadonEditor::Model::ResourceEvent& resource_event)
            {
                switch (resource_event.type)
                {
                case VadonEditor::Model::ResourceEventType::EDITED:
                {
                    Vadon::Model::ResourceSystem& resource_system = get_engine_core().get_system<Vadon::Model::ResourceSystem>();
                    Vadon::Model::ResourceHandle resource_handle = resource_system.find_resource(resource_event.resource);
                    VADON_ASSERT(resource_handle.is_valid() == true, "Resource not found!");

                    const Vadon::Model::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
                    if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<GlobalConfiguration>(), resource_info.type_id))
                    {
                        // Global config resource, check if it's the one in the current project config
                        const Vadon::Core::Project& active_project = m_common_editor.get_project_manager().get_active_project();
                        if (active_project.custom_data_resource_id == resource_event.resource)
                        {
                            m_core->update_global_config(GlobalConfigurationID::from_resource_id(resource_event.resource));
                            m_render.update_editor_layer();
                        }
                    }
                }
                break;
                }
            }
        );

        register_type_metadata(m_common_editor.get_metadata_registry());

        if (m_common_editor.load_project(project_path) == false)
        {
            return false;
        }

        if (project_loaded() == false)
        {
            return false;
        }

        return true;
    }

    void Editor::update()
    {
        TimePoint current_time = Clock::now();
        const float delta_time = std::chrono::duration_cast<Duration>(current_time - m_last_frame_time).count();

        // Limit framerate (input response is better this way)
        if (delta_time < (1.0f / 60.0f))
        {
            std::this_thread::yield();
            return;
        }

        m_delta_time = delta_time;
        m_last_frame_time = current_time;

        m_render.update();

        // Check project state
        VadonEditor::Core::ProjectManager& project_manager = m_common_editor.get_project_manager();
        if (project_manager.is_project_loaded() == false)
        {
            // No project loaded yet, so we'll have nothing to process
            return;
        }

        // Update subsystems
        m_view.update();
        m_ui.update();
        m_platform.update();
    }

    void Editor::shutdown()
    {
        // TODO: shut down demo subsystems (clean up resources, etc.)
        m_common_editor.shutdown();
        m_engine_core->shutdown();
    }

    void Editor::process_message_from_editor(const char* data, size_t size)
    {
        ::Vadon::Foundation::EditorMessageReader message_reader(data, size);
        switch (message_reader.get_current_category())
        {
        case ::Vadon::Foundation::EditorMessageCategory::PLUGIN:
        {
            const char* message_data = message_reader.get_current_message_data();
            const ::Vadon::Foundation::EditorPluginMessageHeader* plugin_message_header = reinterpret_cast<const ::Vadon::Foundation::EditorPluginMessageHeader*>(message_data);
            if (plugin_message_header->plugin_type != ::Vadon::Foundation::EditorPluginMessageSource::SIMULATOR)
            {
                return;
            }

            switch (plugin_message_header->message_type)
            {
            case ::Vadon::Foundation::EditorPluginMessageType::PLUGIN_INIT:
                break;
            case ::Vadon::Foundation::EditorPluginMessageType::PLUGIN_SHUTDOWN:
                break;
            }
        }
        break;
        }

        m_platform.process_message(data, size);
        m_render.process_message(data, size);
        m_common_editor.process_message(data, size);
    }

    void Editor::editor_connected()
    {
        m_platform.editor_connected();
    }

    void Editor::editor_disconnected()
    {
        // TODO
    }

    const ::Vadon::Foundation::TypeMetadataRegistry& Editor::get_metadata_registry() const
    {
        return m_common_editor.get_metadata_registry();
    }

    bool Editor::project_loaded()
    {
        // Retrieve config data to make it available to subsystems
        const Vadon::Core::Project& active_project = m_common_editor.get_project_manager().get_active_project();

        m_core = std::make_unique<Core>(*m_engine_core);
        if (m_core->initialize(active_project) == false)
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