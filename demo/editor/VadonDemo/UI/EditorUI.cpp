#include <VadonDemo/UI/EditorUI.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Core/Editor.hpp>
#include <VadonDemo/UI/Component.hpp>

#include <VadonEditor/Scene/SceneSystem.hpp>

#include <Vadon/ECS/World/World.hpp>

namespace VadonDemo::UI
{
    EditorUI::EditorUI(Core::Editor& editor)
        : m_editor(editor)
        , m_entities_dirty(false)
    { }

	bool EditorUI::initialize()
	{
        VadonEditor::Scene::SceneSystem& editor_scene_system = m_editor.get_common_editor().get_scene_system();

        editor_scene_system.add_entity_event_callback(
            [this](const VadonEditor::Scene::EntityEvent& entity_event)
            {
                switch (entity_event.type)
                {
                case VadonEditor::Scene::EntityEventType::ADDED:
                    init_entity(entity_event.entity);
                    break;
                case VadonEditor::Scene::EntityEventType::REMOVED:
                    remove_entity(entity_event.entity);
                    break;
                }
            }
        );

        editor_scene_system.add_component_event_callback(
            [this](const VadonEditor::Scene::ComponentEvent& component_event)
            {
                if (component_event.component_type == Vadon::Utilities::TypeRegistry::get_type_id<Base>())
                {
                    switch (component_event.type)
                    {
                    case VadonEditor::Scene::ComponentEventType::ADDED:
                        init_entity(component_event.owner);
                        break;
                    case VadonEditor::Scene::ComponentEventType::EDITED:
                        update_entity(component_event.owner);
                        break;
                    case VadonEditor::Scene::ComponentEventType::REMOVED:
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
                    if (component_event.type == VadonEditor::Scene::ComponentEventType::ADDED)
                    {
                        update_entity(component_event.owner);
                    }
                }
            }
        );

        return true;
	}

    void EditorUI::register_type_metadata()
    {

    }

    void EditorUI::update()
    {
        update_dirty_entities();
    }

    void EditorUI::update_dirty_entities()
    {
        if (m_entities_dirty == false)
        {
            return;
        }

        m_entities_dirty = false;

        Vadon::ECS::World& ecs_world = m_editor.get_ecs_world();
        auto base_ui_query = ecs_world.get_component_manager().run_component_query<Base&>();

        UI& common_ui = m_editor.get_core().get_ui();

        for (auto base_it = base_ui_query.get_iterator(); base_it.is_valid() == true; base_it.next())
        {
            auto current_base_component = base_it.get_component<Base>();

            if (current_base_component->dirty == false)
            {
                continue;
            }

            // Update draw data (if applicable)
            common_ui.update_ui_element(ecs_world, base_it.get_entity());

            // Unset the flag
            current_base_component->dirty = false;
        }
    }

    void EditorUI::init_entity(Vadon::ECS::EntityHandle entity)
    {
        // TODO: should we do something different on init?
        update_entity(entity);
    }

    void EditorUI::update_entity(Vadon::ECS::EntityHandle entity)
    {
        Vadon::ECS::World& ecs_world = m_editor.get_ecs_world();

        // Make sure we at least have a base UI component!
        auto base_component = ecs_world.get_component_manager().get_component<Base>(entity);
        if (base_component.is_valid() == false)
        {
            return;
        }

        // Flag entity so it gets updated
        base_component->dirty = true;
        m_entities_dirty = true;
    }

    void EditorUI::remove_entity(Vadon::ECS::EntityHandle entity)
    {
        Vadon::ECS::World& ecs_world = m_editor.get_ecs_world();
        m_editor.get_core().get_ui().remove_ui_element(ecs_world, entity);
    }
}