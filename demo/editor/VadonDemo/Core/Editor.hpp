#ifndef VADONDEMO_CORE_EDITOR_HPP
#define VADONDEMO_CORE_EDITOR_HPP
#include <VadonEditor/Core/Editor.hpp>

#include <VadonDemo/Platform/EditorPlatform.hpp>
#include <VadonDemo/Render/EditorRender.hpp>
#include <VadonDemo/UI/EditorUI.hpp>
#include <VadonDemo/View/EditorView.hpp>
namespace Vadon::Core
{
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

    class Editor : public VadonEditor::Core::Editor
    {
    public:
        Editor(Vadon::Core::EngineEnvironment& environment);
        ~Editor();

        const VadonEditor::Model::Scene* find_entity_scene(Vadon::ECS::EntityHandle entity);

        Core& get_core() { return *m_core; }

        Platform::EditorPlatform& get_platform() { return m_platform; }
        Render::EditorRender& get_render() { return m_render; }
    protected:
        bool initialize(int argc, char* argv[]) override;
        bool project_loaded() override;
        void update() override;
    private:
        std::unique_ptr<VadonDemo::Core::Core> m_core;

        Platform::EditorPlatform m_platform;
        Render::EditorRender m_render;
        UI::EditorUI m_ui;
        View::EditorView m_view;
    };
}
#endif