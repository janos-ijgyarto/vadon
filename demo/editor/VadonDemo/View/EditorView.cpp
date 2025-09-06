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

#include <Vadon/Render/GraphicsAPI/Resource/ResourceSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

namespace VadonDemo::View
{
    EditorView::EditorView(Core::Editor& editor)
        : m_editor(editor)
    {

    }

	bool EditorView::initialize()
	{
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_common_editor().get_system<VadonEditor::Model::ModelSystem>();
        VadonEditor::Model::SceneSystem& editor_scene_system = editor_model.get_scene_system();

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
                if (component_event.component_type == Vadon::Utilities::TypeRegistry::get_type_id<RenderComponent>())
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
                else if ((component_event.component_type == Vadon::Utilities::TypeRegistry::get_type_id<Model::Transform2D>())
                    || (component_event.component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::View::TransformComponent>())
                    || (component_event.component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::View::ModelTransformComponent>())
                    )
                {
                    VadonEditor::Model::ModelSystem& editor_model = m_editor.get_common_editor().get_system<VadonEditor::Model::ModelSystem>();
                    Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

                    m_editor.get_core().get_view().update_entity_transform(ecs_world, component_event.owner);
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

        VadonEditor::Model::ResourceSystem& editor_resource_system = editor_model.get_resource_system();
        editor_resource_system.register_edit_callback(
            [this, &editor_resource_system](Vadon::Scene::ResourceID resource_id)
            {
                // TODO: implement a system for tracking resource references!
                // When it's modified in the editor, we need to know what was previously referenced and de-reference it
                // Brute-force solution: LUT where resource uses some unique key to track what it referenced previously
                // - On type registry, also register a function which handles checking references. This is later found using resource ID

                // NOTE: the above is mostly relevant for the Editor where anything can change at any time
                // In-game a resource will have "static" data, so we can use much simpler reference tracking (each client adds ref to resource, deallocate resource once last client dereferences)
                resource_edited(resource_id);
            }
        );

        return true;
	}

    void EditorView::update()
    {
        update_dirty_entities();

        VadonEditor::View::ViewModel& view_model = m_editor.get_common_editor().get_system<VadonEditor::View::ViewSystem>().get_view_model();
        VadonEditor::Model::Scene* active_scene = view_model.get_active_scene();

        if (active_scene == nullptr)
        {
            // Nothing to do
            return;
        }

        // Control camera per-scene
        update_camera(active_scene);
    }

    void EditorView::update_dirty_entities()
    {
        VadonEditor::Core::Editor& common_editor = m_editor.get_common_editor();
        VadonEditor::Model::ModelSystem& editor_model = common_editor.get_system<VadonEditor::Model::ModelSystem>();

        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();
        Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
        auto dirty_entity_query = component_manager.run_component_query<EntityDirtyTag&, RenderComponent*>();

        VadonDemo::View::View& common_view = m_editor.get_core().get_view();

        std::vector<Vadon::ECS::EntityHandle> dirty_entities;

        for (auto dirty_entity_it = dirty_entity_query.get_iterator(); dirty_entity_it.is_valid() == true; dirty_entity_it.next())
        {
            dirty_entities.push_back(dirty_entity_it.get_entity());

            auto dirty_entity_tuple = dirty_entity_it.get_tuple();
            RenderComponent* render_component = std::get<RenderComponent*>(dirty_entity_tuple);

            if (render_component != nullptr)
            {
                // Make sure the resource is up-to-date
                load_render_resource(render_component->resource);
            }

            // Attempt to redraw based on resource type
            common_view.update_entity_draw_data(ecs_world, dirty_entity_it.get_entity());

            // Remove the dirty tag
            component_manager.set_entity_tag<EntityDirtyTag>(dirty_entity_it.get_entity(), false);
        }
    }

    void EditorView::init_entity(Vadon::ECS::EntityHandle entity)
    {
        // TODO: should we do something different on init?
        update_entity(entity);
    }

    void EditorView::update_entity(Vadon::ECS::EntityHandle entity)
    {
        // Try to update the draw data
        // If it fails, we can just try again the next time the components/resources are updated
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_common_editor().get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
        RenderComponent* render_component = component_manager.get_component<RenderComponent>(entity);
        if (render_component == nullptr)
        {
            return;
        }

        component_manager.set_entity_tag<EntityDirtyTag>(entity, true);
    }

    void EditorView::remove_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_common_editor().get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        m_editor.get_core().get_view().remove_entity(ecs_world, entity);
    }

    void EditorView::resource_edited(Vadon::Scene::ResourceID resource_id)
    {   
        Vadon::Scene::ResourceSystem& resource_system = m_editor.get_common_editor().get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
        Vadon::Scene::ResourceHandle resource_handle = resource_system.find_resource(resource_id);
        VADON_ASSERT(resource_handle.is_valid() == true, "Resource not found!");

        const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);

        if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<RenderResource>(), resource_info.type_id))
        {
            render_resource_edited(RenderResourceID::from_resource_id(resource_id));
        }
        else if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::Render::TextureResource>())
        {
            texture_resource_edited(VadonDemo::Render::TextureResourceID(resource_id));
        }
    }

    void EditorView::render_resource_edited(RenderResourceID view_render_resource)
    {
        // First reset the resource
        VadonDemo::View::View& common_view = m_editor.get_core().get_view();
        common_view.reset_resource_data(view_render_resource);

        // Tag all entities that use this resource
        VadonEditor::Core::Editor& common_editor = m_editor.get_common_editor();
        VadonEditor::Model::ModelSystem& editor_model = common_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
        auto view_query = component_manager.run_component_query<RenderComponent&, VadonDemo::Render::CanvasComponent&>();

        for (auto view_it = view_query.get_iterator(); view_it.is_valid() == true; view_it.next())
        {
            auto view_tuple = view_it.get_tuple();
            RenderComponent& view_render_component = std::get<RenderComponent&>(view_tuple);

            if (view_render_component.resource != view_render_resource)
            {
                continue;
            }

            component_manager.set_entity_tag<EntityDirtyTag>(view_it.get_entity(), true);
        }
    }

    void EditorView::texture_resource_edited(VadonDemo::Render::TextureResourceID texture_id)
    {
        // Run a query to find all entities that use this texture
        // Tag as "dirty" so they get updated
        VadonEditor::Core::Editor& common_editor = m_editor.get_common_editor();
        Vadon::Core::EngineCoreInterface& engine_core = common_editor.get_engine_core();
        Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

        VadonDemo::View::View& common_view = m_editor.get_core().get_view();

        VadonEditor::Model::ModelSystem& editor_model = common_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
        auto view_query = component_manager.run_component_query<RenderComponent&, VadonDemo::Render::CanvasComponent&>();

        for (auto view_it = view_query.get_iterator(); view_it.is_valid() == true; view_it.next())
        {
            auto view_tuple = view_it.get_tuple();
            RenderComponent& current_view_component = std::get<RenderComponent&>(view_tuple);

            if (current_view_component.resource.is_valid() == false)
            {
                continue;
            }

            const RenderResourceHandle view_render_resource_handle = common_view.load_render_resource(current_view_component.resource);
            if (resource_system.get_resource_info(view_render_resource_handle).type_id != Vadon::Utilities::TypeRegistry::get_type_id<Sprite>())
            {
                continue;
            }

            const Sprite* sprite_resource = resource_system.get_resource<Sprite>(SpriteResourceHandle::from_resource_handle(view_render_resource_handle));
            if (sprite_resource->texture != texture_id)
            {
                continue;
            }

            // Reset the sprite that uses this texture
            common_view.reset_resource_data(current_view_component.resource);

            // Mark the entity as "dirty"
            component_manager.set_entity_tag<EntityDirtyTag>(view_it.get_entity(), true);
        }
    }

    void EditorView::load_render_resource(RenderResourceID view_render_resource)
    {
        if (view_render_resource.is_valid() == false)
        {
            return;
        }

        Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_common_editor().get_engine_core();
        Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

        VadonDemo::View::View& common_view = m_editor.get_core().get_view();
        const RenderResourceHandle view_render_resource_handle = common_view.load_render_resource(view_render_resource);

        const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(view_render_resource_handle);
        if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<Sprite>())
        {
            // Try to load the texture data for the sprite
            const Sprite* sprite_resource = resource_system.get_resource(SpriteResourceHandle::from_resource_handle(view_render_resource_handle));
            m_editor.get_render().load_texture_resource_data(sprite_resource->texture);
        }

        common_view.load_resource_data(view_render_resource);
    }

    void EditorView::update_camera(VadonEditor::Model::Scene* active_scene)
    {
        VadonApp::Core::Application& engine_app = m_editor.get_common_editor().get_engine_app();
        if (Vadon::Utilities::to_bool(engine_app.get_system<VadonApp::UI::Developer::GUISystem>().get_io_flags() &
            (VadonApp::UI::Developer::GUISystem::IOFlags::KEYBOARD_CAPTURE | VadonApp::UI::Developer::GUISystem::IOFlags::MOUSE_CAPTURE)) == true)
        {
            // Dev GUI is capturing input
            return;
        }

        Vadon::Math::Vector2 camera_velocity = Vadon::Math::Vector2_Zero;
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