#include <VadonDemo/Core/Editor.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Model/Model.hpp>

#include <VadonDemo/Network/Message/MessageSerializer.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>
#include <VadonEditor/Core/TypeInfo/MetadataRegistry.hpp>

#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Core/Core.hpp>
#include <Vadon/Core/CoreConfiguration.hpp>
#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/System/CommandLine/Parser.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Simulator.hpp>
#include <Vadon/Foundation/Editor/Simulator/SimulatorInterface.hpp>

#include <filesystem>
#include <thread>

namespace VadonDemo::Core
{
    void Logger::log_message(std::string_view message)
    {
        Vadon::Core::DefaultLogger::log_message(message);
        dispatch_network_log_message(::Vadon::Foundation::EditorSimulatorMessageLog::INFO, message);
    }

    void Logger::log_warning(std::string_view message)
    {
        Vadon::Core::DefaultLogger::log_warning(message);
        dispatch_network_log_message(::Vadon::Foundation::EditorSimulatorMessageLog::WARNING, message);
    }

    void Logger::log_error(std::string_view message)
    {
        Vadon::Core::DefaultLogger::log_error(message);
        dispatch_network_log_message(::Vadon::Foundation::EditorSimulatorMessageLog::ERROR, message);
    }

    void Logger::dispatch_network_log_message(::Vadon::Foundation::EditorSimulatorMessageLog::Type type, std::string_view message)
    {
        // Trim trailing newline
        // FIXME: make this more robust!
        std::string_view message_trimmed = message;
        if (message_trimmed.back() == '\n')
        {
            message_trimmed = message.substr(0, message.length() - 1);
        }

        if (message_trimmed.empty())
        {
            return;
        }

        ::Vadon::Foundation::EditorSimulatorMessageLog simulator_log_message;
        simulator_log_message.message_type = Vadon::Foundation::EditorSimulatorMessageType::SIMULATOR_LOG;
        simulator_log_message.log_type = type;
        simulator_log_message.length = static_cast<::Vadon::Foundation::uint32>(message_trimmed.size());

        Network::MessageSerializer message_serializer;
        char* message_data = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::SIMULATOR, sizeof(::Vadon::Foundation::EditorSimulatorMessageLog) + message_trimmed.size());

        memcpy(message_data, &simulator_log_message, sizeof(::Vadon::Foundation::EditorSimulatorMessageLog));
        memcpy(message_data + sizeof(::Vadon::Foundation::EditorSimulatorMessageLog), message_trimmed.data(), message_trimmed.size());

        m_editor.get_simulator().dispatch_message_to_editor(message_serializer.get_buffer().data(), message_serializer.get_buffer().size());
    }

    Editor::Editor(::Vadon::Foundation::EditorSimulatorInterface& simulator_interface)
        : ::Vadon::Foundation::EditorSimulatorPluginInterface(simulator_interface)
        , m_engine_core(Vadon::Core::create_engine_core())
        , m_common_editor(*m_engine_core)
        , m_logger(*this)
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
        editor_resource_system.register_edit_callback(
            [this](Vadon::Model::ResourceID resource_id, ::Vadon::Foundation::UUID /*property_id*/)
            {
                Vadon::Model::ResourceSystem& resource_system = get_engine_core().get_system<Vadon::Model::ResourceSystem>();
                Vadon::Model::ResourceHandle resource_handle = resource_system.find_resource(resource_id);
                VADON_ASSERT(resource_handle.is_valid() == true, "Resource not found!");

                const Vadon::Model::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
                if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<GlobalConfiguration>(), resource_info.type_id))
                {
                    // Global config resource, check if it's the one in the current project config
                    const Vadon::Core::Project& active_project = m_common_editor.get_project_manager().get_active_project();
                    if (active_project.custom_data_id == resource_id)
                    {
                        m_core->update_global_config(GlobalConfigurationID::from_resource_id(resource_id));
                        m_render.update_editor_layer();
                    }
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
        case ::Vadon::Foundation::EditorMessageCategory::SIMULATOR:
        {
            const char* message_data = message_reader.get_current_message_data();
            const ::Vadon::Foundation::EditorSimulatorMessageHeader* simulator_message_header = reinterpret_cast<const ::Vadon::Foundation::EditorSimulatorMessageHeader*>(message_data);
            switch (simulator_message_header->message_type)
            {
            case ::Vadon::Foundation::EditorSimulatorMessageType::SIMULATOR_INIT:
                break;
            case ::Vadon::Foundation::EditorSimulatorMessageType::SIMULATOR_SHUTDOWN:
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