#ifndef VADONDEMO_CORE_EDITOR_HPP
#define VADONDEMO_CORE_EDITOR_HPP
#include <VadonDemo/Platform/EditorPlatform.hpp>
#include <VadonDemo/Render/EditorRender.hpp>
#include <VadonDemo/UI/EditorUI.hpp>
#include <VadonDemo/View/EditorView.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <Vadon/Foundation/Editor/Simulator/PluginInterface.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Simulator.hpp>

#include <Vadon/Core/Core.hpp>
#include <Vadon/Core/Logger.hpp>
#include <Vadon/ECS/World/World.hpp>

#include <chrono>

namespace Vadon::Core
{
    class EngineCoreInterface;
    class EngineEnvironment;
}
namespace VadonDemo::Core
{
    class Core;
    class EditorPluginInterface;

    class Editor;

    class Logger : public Vadon::Core::DefaultLogger
    {
    public:
        Logger(Editor& editor) : m_editor(editor) {}

        void log_message(std::string_view message) override;
        void log_warning(std::string_view message) override;
        void log_error(std::string_view message) override;
    private:
        void dispatch_network_log_message(::Vadon::Foundation::EditorSimulatorMessageLog::Type type, std::string_view message);

        Editor& m_editor;
    };

    class Editor : public ::Vadon::Foundation::EditorSimulatorPluginInterface
    {
    public:
        Editor(::Vadon::Foundation::EditorSimulatorInterface& simulator_interface);
        ~Editor();

        static void init_environment(Vadon::Core::EngineEnvironment& environment);

        VadonEditor::Core::Editor& get_common_editor() { return m_common_editor; }

        Vadon::Core::EngineCoreInterface& get_engine_core() { return *m_engine_core; }

        Core& get_core() { return *m_core; }

        Vadon::ECS::World& get_ecs_world() { return m_ecs_world; }

        Platform::EditorPlatform& get_platform() { return m_platform; }
        Render::EditorRender& get_render() { return m_render; }
        View::EditorView& get_view() { return m_view; }

        float get_delta_time() const { return m_delta_time; }

        bool initialize(const char* project_path) override;
        void update() override;
        void shutdown() override;

        void process_message_from_editor(const char* data, size_t size) override;

        void editor_connected() override;
        void editor_disconnected() override;

        const ::Vadon::Foundation::TypeMetadataRegistry& get_metadata_registry() const override;

        static void register_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& registry);
    private:
        using Clock = std::chrono::steady_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Duration = std::chrono::duration<float>;

        void update_subsystems();

        bool project_loaded();

        Vadon::Core::EngineCorePtr m_engine_core;
        VadonEditor::Core::Editor m_common_editor;
        Logger m_logger;

        std::unique_ptr<VadonDemo::Core::Core> m_core;

        Vadon::ECS::World m_ecs_world;

        Platform::EditorPlatform m_platform;
        Render::EditorRender m_render;
        UI::EditorUI m_ui;
        View::EditorView m_view;

        bool m_running;
        float m_delta_time;
        TimePoint m_last_frame_time;
    };
}
#endif