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
        VadonEditor::Model::SceneSystem& editor_scene_system = m_editor.get_system<VadonEditor::Model::ModelSystem>().get_scene_system();

        editor_scene_system.add_entity_event_callback(
            [this](const VadonEditor::Model::EntityEvent& entity_event)
            {
                switch (entity_event.type)
                {
                case VadonEditor::Model::EntityEventType::ADDED:
                    init_entity(entity_event.entity);
                    break;
                case VadonEditor::Model::EntityEventType::REMOVED:
                    remove_entity(entity_event.entity);
                    break;
                }
            }
        );

        editor_scene_system.add_component_event_callback(
            [this](const VadonEditor::Model::ComponentEvent& component_event)
            {
                if (component_event.component_type == Vadon::Utilities::TypeRegistry::get_type_id<Base>())
                {
                    switch (component_event.type)
                    {
                    case VadonEditor::Model::ComponentEventType::ADDED:
                        init_entity(component_event.owner);
                        break;
                    case VadonEditor::Model::ComponentEventType::EDITED:
                        update_entity(component_event.owner);
                        break;
                    case VadonEditor::Model::ComponentEventType::REMOVED:
                        remove_entity(component_event.owner);
                        break;
                    }
                }
                else if (component_event.component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::UI::Frame>()
                    || component_event.component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::UI::Text>())
                {
                    update_entity(component_event.owner);
                }
                else if (component_event.component_type == Vadon::Utilities::TypeRegistry::get_type_id<Render::CanvasComponent>())
                {
                    // TODO: could also make this event-driven
                    // EditorRender initializes CanvasComponent, fires event asking for it to be initialized
                    // Other subsystems can add listener and add draw data
                    if (component_event.type == VadonEditor::Model::ComponentEventType::ADDED)
                    {
                        update_entity(component_event.owner);
                    }
                }
            }
        );

        return true;
	}

    void EditorUI::update()
    {
        // TODO: anything?
    }

    void EditorUI::init_entity(Vadon::ECS::EntityHandle entity)
    {
        // TODO: should we do something different on init?
        update_entity(entity);
    }

    void EditorUI::update_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        // Make sure we at least have a base UI component!
        Base* base_component = ecs_world.get_component_manager().get_component<Base>(entity);
        if (base_component == nullptr)
        {
            return;
        }

        // Make sure render component is initialized
        m_editor.get_render().init_entity(entity);

        m_editor.get_core().get_ui().update_ui_element(ecs_world, entity);
    }

    void EditorUI::remove_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        m_editor.get_core().get_ui().remove_ui_element(ecs_world, entity);
    }
}