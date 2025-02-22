#include <VadonDemo/View/EditorView.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Core/Editor.hpp>

#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Platform/EditorPlatform.hpp>
#include <VadonDemo/View/Component.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>
#include <VadonEditor/View/ViewSystem.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/Platform/Input/InputSystem.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/CoreInterface.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

namespace VadonDemo::View
{

    EditorView::EditorView(Core::Editor& editor)
        : m_editor(editor)
    {

    }

	bool EditorView::initialize()
	{
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
        component_manager.register_event_callback<VadonDemo::Model::Transform2D>(
            [this, &ecs_world](const Vadon::ECS::ComponentEvent& component_event)
            {
                // Can update transform directly
                m_editor.get_core().get_view().update_entity_transform(ecs_world, component_event.owner);
            }
        );

        component_manager.register_event_callback<VadonDemo::View::ViewComponent>(
            [this](const Vadon::ECS::ComponentEvent& component_event)
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

        component_manager.register_event_callback<VadonDemo::Render::CanvasComponent>(
            [this](const Vadon::ECS::ComponentEvent& component_event)
            {
                update_entity(component_event.owner);
            }
        );

        // FIXME: also need to make sure we remove the scene canvas context when scene is closed!
        VadonEditor::Model::SceneSystem& scene_system = editor_model.get_scene_system();
        scene_system.register_edit_callback(
            [this, &ecs_world](Vadon::ECS::EntityHandle entity, Vadon::Utilities::TypeID component_type)
            {
                View& view = m_editor.get_core().get_view();
                if (component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::Model::Transform2D>())
                {
                    view.update_entity_transform(ecs_world, entity);
                }
                else if ((component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::View::ViewComponent>())
                    || (component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::Render::CanvasComponent>()))
                {
                    update_entity(entity);
                }
            }
        );

        VadonEditor::Model::ResourceSystem& editor_resource_system = editor_model.get_resource_system();
        editor_resource_system.register_edit_callback(
            [this, &editor_resource_system](Vadon::Scene::ResourceID resource_id)
            {
                Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
                Vadon::Scene::ResourceHandle resource_handle = resource_system.find_resource(resource_id);
                VADON_ASSERT(resource_handle.is_valid() == true, "Resource not found!");

                const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
                if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::View::ViewResource>(), resource_info.type_id))
                {
                    update_resource(VadonDemo::View::ViewResourceHandle::from_resource_handle(resource_handle));
                }
            }
        );

        return true;
	}

    void EditorView::update()
    {
        if (m_deferred_update_queue.empty() == false)
        {
            for (Vadon::ECS::EntityHandle current_entity : m_deferred_update_queue)
            {
                deferred_update_entity(current_entity);
            }

            m_deferred_update_queue.clear();
        }

        VadonEditor::View::ViewModel& view_model = m_editor.get_system<VadonEditor::View::ViewSystem>().get_view_model();
        VadonEditor::Model::Scene* active_scene = view_model.get_active_scene();

        if (active_scene == nullptr)
        {
            // Nothing to do
            return;
        }

        // Control camera per-scene
        update_camera(active_scene);
    }

    void EditorView::update_entity(Vadon::ECS::EntityHandle entity)
    {
        // Defer until next update
        // By that point, we expect all the relevant changes to have happened
        auto entity_it = std::find(m_deferred_update_queue.begin(), m_deferred_update_queue.end(), entity);
        if (entity_it == m_deferred_update_queue.end())
        {
            m_deferred_update_queue.push_back(entity);
        }
    }

    void EditorView::deferred_update_entity(Vadon::ECS::EntityHandle entity)
    {
        // Try to update the draw data
        // If it fails, we can just try again the next time the components/resources are updated
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        // Make sure the resource is up to date
        ViewComponent* view_component = ecs_world.get_component_manager().get_component<ViewComponent>(entity);
        if (view_component == nullptr)
        {
            return;
        }

        if (view_component->resource.is_valid() == true)
        {
            update_resource(view_component->resource);
        }

        m_editor.get_core().get_view().update_entity_draw_data(ecs_world, entity);
    }

    void EditorView::remove_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        m_editor.get_core().get_view().remove_entity(ecs_world, entity);
    }

    void EditorView::update_resource(VadonDemo::View::ViewResourceHandle resource_handle)
    {
        Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
        const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);

        if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::View::Sprite>(), resource_info.type_id))
        {
            load_sprite_resource(VadonDemo::View::SpriteResourceHandle::from_resource_handle(resource_handle));
        }

        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        m_editor.get_core().get_view().update_resource(editor_model.get_ecs_world(), resource_handle);
    }

    void EditorView::load_sprite_resource(VadonDemo::View::SpriteResourceHandle sprite_handle)
    {
        // TODO: implement general system for loading textures as resources!
        Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
        VadonDemo::View::Sprite* sprite_resource = resource_system.get_resource(sprite_handle);

        if (sprite_resource->texture_path.empty())
        {
            sprite_resource->texture_srv.invalidate();
            return;
        }

        Render::TextureResource* sprite_texture = m_editor.get_render().get_texture_resource(sprite_resource->texture_path);
        if (sprite_texture != nullptr)
        {
            sprite_resource->texture_srv = sprite_texture->srv;
        }
        else
        {
            sprite_resource->texture_srv.invalidate();
        }
    }

    void EditorView::update_camera(VadonEditor::Model::Scene* active_scene)
    {
        VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
        if (Vadon::Utilities::to_bool(engine_app.get_system<VadonApp::UI::Developer::GUISystem>().get_io_flags() &
            (VadonApp::UI::Developer::GUISystem::IOFlags::KEYBOARD_CAPTURE | VadonApp::UI::Developer::GUISystem::IOFlags::MOUSE_CAPTURE)) == true)
        {
            // Dev GUI is capturing input
            return;
        }

        Vadon::Utilities::Vector2 camera_velocity = Vadon::Utilities::Vector2_Zero;
        float camera_zoom = 0.0f;

        VadonApp::Platform::InputSystem& input_system = engine_app.get_system<VadonApp::Platform::InputSystem>();
        Platform::EditorPlatform& editor_platform = m_editor.get_platform();
        if (input_system.is_action_pressed(editor_platform.get_action(Platform::EditorInputAction::CAMERA_LEFT)) == true)
        {
            camera_velocity.x = -1.0f;
        }
        else if (input_system.is_action_pressed(editor_platform.get_action(Platform::EditorInputAction::CAMERA_RIGHT)) == true)
        {
            camera_velocity.x = 1.0f;
        }

        if (input_system.is_action_pressed(editor_platform.get_action(Platform::EditorInputAction::CAMERA_UP)) == true)
        {
            camera_velocity.y = 1.0f;
        }
        else if (input_system.is_action_pressed(editor_platform.get_action(Platform::EditorInputAction::CAMERA_DOWN)) == true)
        {
            camera_velocity.y = -1.0f;
        }

        camera_zoom = input_system.get_action_strength(editor_platform.get_action(Platform::EditorInputAction::CAMERA_ZOOM));

        VadonDemo::Render::CanvasContextHandle scene_context = m_editor.get_render().get_scene_canvas_context(active_scene);
        Vadon::Render::Canvas::RenderContext& render_context = m_editor.get_core().get_render().get_context(scene_context);

        render_context.camera.view_rectangle.position += m_editor.get_delta_time() * 200 * camera_velocity;
        render_context.camera.zoom = std::clamp(render_context.camera.zoom + m_editor.get_delta_time() * camera_zoom * 10.0f, 0.1f, 10.0f);
    }
}