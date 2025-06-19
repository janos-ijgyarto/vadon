#ifndef VADONDEMO_CORE_EDITOR_HPP
#define VADONDEMO_CORE_EDITOR_HPP
#include <VadonEditor/Core/Editor.hpp>

#include <VadonDemo/Platform/EditorPlatform.hpp>
#include <VadonDemo/Render/EditorRender.hpp>
#include <VadonDemo/UI/EditorUI.hpp>
#include <VadonDemo/View/EditorView.hpp>

#include <Vadon/Core/Core.hpp>

namespace Vadon::Core
{
    class EngineCoreInterface;
    class EngineEnvironment;
}
namespace VadonEditor::Model
{
    class Scene;
}
namespace VadonDemo::Model
{
    class Model;
}
namespace VadonDemo::Core
{
    class Core;

    class Editor
    {
    public:
        Editor(Vadon::Core::EngineEnvironment& environment);
        ~Editor();

        // TODO: revise this so we don't expose to subsystems?
        int execute(int argc, char* argv[]);

        VadonEditor::Core::Editor& get_common_editor() { return m_common_editor; }

        const VadonEditor::Model::Scene* find_entity_scene(Vadon::ECS::EntityHandle entity);

        Core& get_core() { return *m_core; }

        Platform::EditorPlatform& get_platform() { return m_platform; }
        Render::EditorRender& get_render() { return m_render; }

        float get_delta_time() const { return m_delta_time; }
    private:
        bool initialize(int argc, char* argv[]);
        void shutdown();

        void begin_frame();
        void update_subsystems();

        bool project_loaded();

        Vadon::Core::EngineCorePtr m_engine_core;
        VadonEditor::Core::Editor m_common_editor;

        std::unique_ptr<VadonDemo::Core::Core> m_core;

        Platform::EditorPlatform m_platform;
        Render::EditorRender m_render;
        UI::EditorUI m_ui;
        View::EditorView m_view;

        bool m_running;
        float m_delta_time;
    };
}
#endif