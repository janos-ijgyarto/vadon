#include <VadonDemo/Core/Editor.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Model/Model.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

namespace VadonDemo::Core
{
    Editor::Editor(Vadon::Core::EngineEnvironment& environment)
        : VadonEditor::Core::Editor(environment)
        , m_platform(*this)
        , m_render(*this)
        , m_ui(*this)
        , m_view(*this)
    {
        VadonDemo::Core::Core::init_environment(environment);
    }

    Editor::~Editor() = default;

    const VadonEditor::Model::Scene* Editor::find_entity_scene(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
        return editor_model.get_scene_system().find_entity_scene(entity);
    }

    bool Editor::initialize(int argc, char* argv[])
    {
        // Run the parent initialization
        if (VadonEditor::Core::Editor::initialize(argc, argv) == false)
        {
            return false;
        }

        m_core = std::make_unique<Core>(get_engine_core());
        if (m_core->initialize() == false)
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

        if (m_view.initialize() == false)
        {
            return false;
        }

        if (m_ui.initialize() == false)
        {
            return false;
        }

        // Add self to editor main loop
        VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
        editor_model.add_callback([this]() { update(); });

        return true;
    }

    void Editor::update()
    {
        // Update subsystems
        m_render.update();
        m_view.update();
        m_ui.update();
    }
}