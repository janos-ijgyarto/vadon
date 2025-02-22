#include <VadonDemo/UI/EditorUI.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Core/Editor.hpp>
#include <VadonDemo/UI/Component.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <Vadon/ECS/World/World.hpp>

namespace VadonDemo::UI
{
    EditorUI::EditorUI(Core::Editor& editor)
        : m_editor(editor)
    { }

	bool EditorUI::initialize()
	{
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

        component_manager.register_event_callback<VadonDemo::UI::Base>(
            [this, &ecs_world](const Vadon::ECS::ComponentEvent& component_event)
            {
                switch (component_event.event_type)
                {
                case Vadon::ECS::ComponentEventType::ADDED:
                    update_entity(component_event.owner);
                    break;
                case Vadon::ECS::ComponentEventType::REMOVED:
                    remove_entity(component_event.owner);
                    break;
                }
            }
        );

        component_manager.register_event_callback<VadonDemo::UI::Frame>(
            [this, &ecs_world](const Vadon::ECS::ComponentEvent& component_event)
            {
                update_entity(component_event.owner);
            }
        );

        component_manager.register_event_callback<VadonDemo::UI::Text>(
            [this, &ecs_world](const Vadon::ECS::ComponentEvent& component_event)
            {
                update_entity(component_event.owner);
            }
        );

        component_manager.register_event_callback<VadonDemo::Render::CanvasComponent>(
            [this, &ecs_world](const Vadon::ECS::ComponentEvent& component_event)
            {
                update_entity(component_event.owner);
            }
        );

        // FIXME: also need to make sure we remove the scene canvas context when scene is closed!
        VadonEditor::Model::SceneSystem& scene_system = editor_model.get_scene_system();
        scene_system.register_edit_callback(
            [this](Vadon::ECS::EntityHandle entity, Vadon::Utilities::TypeID component_type)
            {
                if (component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::UI::Base>()
                    || component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::UI::Frame>()
                    || component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::UI::Text>()
                    || component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::UI::Selectable>()
                    || component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::Render::CanvasComponent>())
                {
                    update_entity(entity);
                }
            }
        );

        return true;
	}

    void EditorUI::update()
    {
        if (m_deferred_update_queue.empty() == false)
        {
            for(Vadon::ECS::EntityHandle current_entity : m_deferred_update_queue)
            {
                deferred_update_entity(current_entity);
            }

            m_deferred_update_queue.clear();
        }
    }

    void EditorUI::update_entity(Vadon::ECS::EntityHandle entity)
    {
        auto entity_it = std::find(m_deferred_update_queue.begin(), m_deferred_update_queue.end(), entity);
        if (entity_it == m_deferred_update_queue.end())
        {
            m_deferred_update_queue.push_back(entity);
        }
    }

    void EditorUI::deferred_update_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        m_editor.get_core().get_ui().update_ui_element(ecs_world, entity);
    }

    void EditorUI::remove_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        m_editor.get_core().get_ui().remove_ui_element(ecs_world, entity);
    }
}