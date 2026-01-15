#include <VadonDemo/Core/Editor.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Model/Model.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>
#include <VadonEditor/Core/TypeInfo/MetadataRegistry.hpp>

#include <VadonEditor/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Core/Core.hpp>
#include <Vadon/Core/CoreConfiguration.hpp>
#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>
#include <Vadon/Utilities/System/CommandLine/Parser.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>

#include <filesystem>
#include <thread>

namespace VadonDemo::Core
{
    Editor::Editor(Vadon::Core::EngineEnvironment& environment, ::Vadon::Foundation::EditorSimulatorInterface& simulator_interface)
        : ::Vadon::Foundation::EditorPluginInterface(simulator_interface)
        , m_engine_core(Vadon::Core::create_engine_core())
        , m_platform(*this)
        , m_render(*this)
        , m_ui(*this)
        , m_view(*this)
        , m_running(true)
        , m_delta_time(0.0f)
    {
        VadonEditor::Core::Editor::init_environment(environment);
        VadonDemo::Core::Core::init_environment(environment);

        m_last_frame_time = Clock::now();
    }

    Editor::~Editor() = default;

    void Editor::shutdown()
    {
        // TODO: shut down demo subsystems (clean up resources, etc.)
        m_common_editor.shutdown();
        m_engine_core->shutdown();
    }

    void Editor::register_type_metadata()
    {
        VadonEditor::Core::MetadataRegistry& metadata_registry = m_common_editor.get_metadata_registry();
        {
            metadata_registry.set_type_metadata(VADON_GET_TYPE_UUID(Vadon::Scene::Resource), "name", "Vadon::Scene::Resource");
            metadata_registry.set_property_metadata(VADON_GET_TYPE_UUID(Vadon::Scene::Resource), VADON_GET_MEMBER_UUID(Vadon::Scene::Resource, name), "name", "Name");
        }
        {
            // TODO: other types!
        }

        m_render.register_type_metadata();
        m_ui.register_type_metadata();
        m_view.register_type_metadata();
    }

    void Editor::update_subsystems()
    {
        m_view.update();
        m_ui.update();
        m_render.update();
    }

    bool Editor::initialize()
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
        VadonEditor::Scene::ResourceSystem& editor_resource_system = m_common_editor.get_resource_system();
        editor_resource_system.register_edit_callback(
            [this](Vadon::Scene::ResourceID resource_id)
            {
                Vadon::Scene::ResourceSystem& resource_system = get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
                Vadon::Scene::ResourceHandle resource_handle = resource_system.find_resource(resource_id);
                VADON_ASSERT(resource_handle.is_valid() == true, "Resource not found!");

                const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
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

        // FIXME: currently we can register the types here, but ideally we should have a system that tracks
        // the dependencies in the type registry and ensures they are registered in the correct order
        VadonDemo::Core::Core::register_types();

        register_type_metadata();

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

    void Editor::process_message_from_editor(const char* data, size_t size)
    {
        m_platform.process_message(data, size);
        m_render.process_message(data, size);
        // TODO: model messages!
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