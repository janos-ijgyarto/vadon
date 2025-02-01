#include <VadonDemo/Model/Model.hpp>
#include <VadonDemo/Model/Component.hpp>

#include <VadonDemo/UI/Component.hpp>
#include <VadonDemo/UI/UI.hpp>

#include <VadonDemo/View/View.hpp>
#include <VadonDemo/View/Component.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/View/ViewSystem.hpp>

#include <VadonEditor/Platform/PlatformInterface.hpp>
#include <VadonEditor/Render/RenderSystem.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/Platform/Input/InputSystem.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/Context.hpp>
#include <Vadon/Render/Canvas/CanvasSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/TextureSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

namespace
{
    // FIXME: should split this up into subsystems!
    class Editor : public VadonEditor::Core::Editor
    {
    public:
        Editor(Vadon::Core::EngineEnvironment& environment)
            : VadonEditor::Core::Editor(environment)
        {
            // Initialize both for editor library and the game-specific model
            VadonDemo::Model::Model::init_engine_environment(environment);
        }
    protected:
        bool initialize(int argc, char* argv[]) override
        {
            // Run the parent initialization
            if (VadonEditor::Core::Editor::initialize(argc, argv) == false)
            {
                return false;
            }

            // Initialize Demo-specific systems
            m_model = std::make_unique<VadonDemo::Model::Model>(get_engine_core());
            if (m_model->initialize() == false)
            {
                return false;
            }

            m_view = std::make_unique<VadonDemo::View::View>(get_engine_core());
            if (m_view->initialize() == false)
            {
                return false;
            }

            m_ui = std::make_unique<VadonDemo::UI::UI>(get_engine_core());
            if (m_ui->initialize() == false)
            {
                return false;
            }

            if (init_renderer() == false)
            {
                return false;
            }

            if (init_model_view() == false)
            {
                return false;
            }

            if (init_platform() == false)
            {
                return false;
            }

            return true;
        }
    private:
        bool init_renderer()
        {
            VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
            VadonEditor::Render::RenderSystem& editor_render = get_system<VadonEditor::Render::RenderSystem>();

            Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

            editor_model.add_callback(
                [this, &editor_model, &editor_render, &ecs_world]()
                {
                    VadonEditor::View::ViewModel& view_model = get_system<VadonEditor::View::ViewSystem>().get_view_model();
                    VadonEditor::Model::Scene* active_scene = view_model.get_active_scene();
                    if (active_scene == nullptr)
                    {
                        return;
                    }

                    if (m_deferred_entity_init_queue.empty() == false)
                    {
                        // FIXME: refactor this!
                        for (size_t index = 0; index < m_deferred_entity_init_queue.size();)
                        {
                            const VadonEditor::Model::Scene* scene = find_entity_scene(m_deferred_entity_init_queue[index]);
                            if (scene != nullptr)
                            {
                                init_view_entity(m_deferred_entity_init_queue[index]);
                                init_ui_entity(m_deferred_entity_init_queue[index]);
                                if (index < (m_deferred_entity_init_queue.size() - 1))
                                {
                                    m_deferred_entity_init_queue[index] = m_deferred_entity_init_queue.back();
                                }
                                m_deferred_entity_init_queue.pop_back();
                                continue;
                            }
                            ++index;
                        }
                    }

                    m_ui->update(ecs_world);

                    Vadon::Render::Canvas::RenderContext& active_scene_context = get_scene_canvas_context(active_scene);

                    // Control camera per-scene
                    Vadon::Utilities::Vector2 camera_velocity = Vadon::Utilities::Vector2_Zero;
                    float camera_zoom = 0.0f;

                    VadonApp::Core::Application& engine_app = get_engine_app();
                    if (Vadon::Utilities::to_bool(engine_app.get_system<VadonApp::UI::Developer::GUISystem>().get_io_flags() & VadonApp::UI::Developer::GUISystem::IOFlags::KEYBOARD_CAPTURE) == false)
                    {
                        VadonApp::Platform::InputSystem& input_system = engine_app.get_system<VadonApp::Platform::InputSystem>();
                        if (input_system.is_action_pressed(m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_LEFT)]) == true)
                        {
                            camera_velocity.x = -1.0f;
                        }
                        else if (input_system.is_action_pressed(m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_RIGHT)]) == true)
                        {
                            camera_velocity.x = 1.0f;
                        }

                        if (input_system.is_action_pressed(m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_UP)]) == true)
                        {
                            camera_velocity.y = 1.0f;
                        }
                        else if (input_system.is_action_pressed(m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_DOWN)]) == true)
                        {
                            camera_velocity.y = -1.0f;
                        }

                        camera_zoom = input_system.get_action_strength(m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_ZOOM)]);
                    }

                    // Update viewport and camera rectangle
                    // FIXME: only update this when it changes!
                    VadonApp::Platform::WindowHandle main_window = get_system<VadonEditor::Platform::PlatformInterface>().get_main_window();
                    const Vadon::Utilities::Vector2i window_size = engine_app.get_system<VadonApp::Platform::PlatformInterface>().get_window_drawable_size(main_window);
                    active_scene_context.viewports.back().render_viewport.dimensions.size = window_size;

                    active_scene_context.camera.view_rectangle.position += get_delta_time() * 200 * camera_velocity;
                    active_scene_context.camera.view_rectangle.size = window_size;
                    active_scene_context.camera.zoom = std::clamp(active_scene_context.camera.zoom + get_delta_time() * camera_zoom * 10.0f, 0.1f, 10.0f);

                    editor_render.enqueue_canvas(active_scene_context);
                }
            );

            return true;
        }

        bool init_model_view()
        {
            VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
            Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

            Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

            component_manager.register_event_callback<VadonDemo::Model::Transform2D>(
                [this, &ecs_world](const Vadon::ECS::ComponentEvent& component_event)
                {
                    m_view->update_view_entity_transform(ecs_world, component_event.owner);
                }
            );

            component_manager.register_event_callback<VadonDemo::View::ViewComponent>(
                [this, &ecs_world](const Vadon::ECS::ComponentEvent& component_event)
                {
                    switch (component_event.event_type)
                    {
                    case Vadon::ECS::ComponentEventType::ADDED:
                        init_view_entity(component_event.owner);
                        break;
                    case Vadon::ECS::ComponentEventType::REMOVED:
                        remove_view_entity(ecs_world, component_event.owner);
                        break;
                    }
                }
            );

            component_manager.register_event_callback<VadonDemo::UI::Base>(
                [this, &ecs_world](const Vadon::ECS::ComponentEvent& component_event)
                {
                    switch (component_event.event_type)
                    {
                    case Vadon::ECS::ComponentEventType::ADDED:
                        init_ui_entity(component_event.owner);
                        break;
                    case Vadon::ECS::ComponentEventType::REMOVED:
                        remove_view_entity(ecs_world, component_event.owner);
                        break;
                    }
                }
            );

            component_manager.register_event_callback<VadonDemo::UI::Frame>(
                [this, &ecs_world](const Vadon::ECS::ComponentEvent& component_event)
                {
                    update_ui_entity(ecs_world, component_event.owner);
                }
            );

            component_manager.register_event_callback<VadonDemo::UI::Text>(
                [this, &ecs_world](const Vadon::ECS::ComponentEvent& component_event)
                {
                    update_ui_entity(ecs_world, component_event.owner);
                }
            );

            // FIXME: also need to make sure we remove the scene canvas context when scene is closed!
            VadonEditor::Model::SceneSystem& scene_system = editor_model.get_scene_system();
            scene_system.register_edit_callback(
                [this](Vadon::ECS::EntityHandle entity, Vadon::Utilities::TypeID component_type)
                {
                    VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
                    Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();
                    if(component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::Model::Transform2D>())
                    {
                        m_view->update_view_entity_transform(ecs_world, entity);
                    }
                    else if (component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::View::ViewComponent>())
                    {
                        // Make sure the resource is up-to-date
                        VadonDemo::View::ViewComponent* view_component = ecs_world.get_component_manager().get_component<VadonDemo::View::ViewComponent>(entity);
                        VADON_ASSERT(view_component != nullptr, "Cannot find View component!");

                        // Make sure the resource is up-to-date
                        init_view_resource(view_component->resource);
                        m_view->update_view_entity_draw_data(ecs_world, entity);
                    }
                    else if (component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::UI::Base>()
                        || component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::UI::Frame>()
                        || component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::UI::Text>()
                        || component_type == Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::UI::Selectable>())
                    {
                        update_ui_entity(ecs_world, entity);
                    }
                }
            );

            VadonEditor::Model::ResourceSystem& editor_resource_system = editor_model.get_resource_system();
            editor_resource_system.register_edit_callback(
                [this, &editor_resource_system](Vadon::Scene::ResourceID resource_id)
                {
                    Vadon::Scene::ResourceSystem& resource_system = get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
                    Vadon::Scene::ResourceHandle resource_handle = resource_system.find_resource(resource_id);
                    VADON_ASSERT(resource_handle.is_valid() == true, "Resource not found!");

                    const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
                    if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::View::ViewResource>(), resource_info.type_id))
                    {
                        update_view_resource(VadonDemo::View::ViewResourceHandle::from_resource_handle(resource_handle));
                    }
                }
            );
            return true;
        }

        bool init_platform()
        {
            VadonApp::Core::Application& engine_app = get_engine_app();
            VadonApp::Platform::InputSystem& input_system = engine_app.get_system<VadonApp::Platform::InputSystem>();

            {
                VadonApp::Platform::InputActionHandle camera_up_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_up" });
                input_system.add_key_entry(camera_up_action, VadonApp::Platform::KeyCode::UP);
                m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_UP)] = camera_up_action;
            }

            {
                VadonApp::Platform::InputActionHandle camera_down_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_down" });
                input_system.add_key_entry(camera_down_action, VadonApp::Platform::KeyCode::DOWN);
                m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_DOWN)] = camera_down_action;
            }

            {
                VadonApp::Platform::InputActionHandle camera_left_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_left" });
                input_system.add_key_entry(camera_left_action, VadonApp::Platform::KeyCode::LEFT);
                m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_LEFT)] = camera_left_action;
            }

            {
                VadonApp::Platform::InputActionHandle camera_right_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_right" });
                input_system.add_key_entry(camera_right_action, VadonApp::Platform::KeyCode::RIGHT);
                m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_RIGHT)] = camera_right_action;
            }

            {
                VadonApp::Platform::InputActionHandle camera_zoom_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_zoom" });
                input_system.add_mouse_wheel_entry(camera_zoom_action);
                m_input_actions[Vadon::Utilities::to_integral(InputAction::CAMERA_ZOOM)] = camera_zoom_action;
            }

            return true;
        }

        VadonEditor::Model::Scene* find_entity_scene(Vadon::ECS::EntityHandle entity)
        {
            VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
            Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

            Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
            Vadon::ECS::EntityHandle root_entity = entity;
            while (true)
            {
                Vadon::ECS::EntityHandle parent = entity_manager.get_entity_parent(root_entity);
                if (parent.is_valid() == false)
                {
                    break;
                }

                root_entity = parent;
            }

            // Check whether we already added the scene that contains this Entity
            // If not, we'll initialize once the scene is selected for rendering
            for (auto& scene_context_pair : m_scene_canvas_contexts)
            {
                if (scene_context_pair.first->get_root()->get_handle() == root_entity)
                {
                    return scene_context_pair.first;
                }
            }

            return nullptr;
        }

        Vadon::Render::Canvas::RenderContext& get_scene_canvas_context(VadonEditor::Model::Scene* scene)
        {
            VADON_ASSERT(scene != nullptr, "Scene must not be null!");

            auto scene_context_it = m_scene_canvas_contexts.find(scene);
            if (scene_context_it == m_scene_canvas_contexts.end())
            {
                // Scene not displayed before, set up new context
                Vadon::Render::Canvas::RenderContext new_context;

                // Set up canvas layers
                {
                    Vadon::ECS::EntityHandle root_entity = scene->get_root()->get_handle();

                    Vadon::Render::Canvas::CanvasSystem& canvas_system = get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();
                    VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
                    Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();
                    {
                        Vadon::Render::Canvas::LayerHandle view_layer = canvas_system.create_layer(Vadon::Render::Canvas::LayerInfo{});

                        VADON_ASSERT(view_layer.is_valid() == true, "Failed to create view canvas layer!");
                        new_context.layers.push_back(view_layer);

                        init_view_entity(ecs_world, root_entity, view_layer);
                    }
                    {
                        Vadon::Render::Canvas::LayerHandle ui_layer = canvas_system.create_layer(Vadon::Render::Canvas::LayerInfo{ .flags = Vadon::Render::Canvas::LayerInfo::Flags::VIEW_AGNOSTIC });

                        VADON_ASSERT(ui_layer.is_valid() == true, "Failed to create UI canvas layer!");
                        new_context.layers.push_back(ui_layer);

                        init_ui_entity(ecs_world, root_entity, ui_layer);
                    }
                }

                // Set up viewport
                {
                    Vadon::Render::RenderTargetSystem& rt_system = get_engine_core().get_system<Vadon::Render::RenderTargetSystem>();

                    Vadon::Render::Canvas::Viewport canvas_viewport;
                    canvas_viewport.render_target = rt_system.get_window_target(get_system<VadonEditor::Render::RenderSystem>().get_render_window());

                    new_context.viewports.push_back(canvas_viewport);
                }

                scene_context_it = m_scene_canvas_contexts.insert(std::make_pair(scene, new_context)).first;
            }

            return scene_context_it->second;
        }

        void init_view_entity(Vadon::ECS::EntityHandle entity)
        {
            VadonEditor::Model::Scene* scene = find_entity_scene(entity);
            if (scene == nullptr)
            {
                // Scene not available yet, add to deferred queue
                m_deferred_entity_init_queue.push_back(entity);
                return;
            }

            // Scene has layer, add entity
            VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
            Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

            auto scene_context_it = m_scene_canvas_contexts.find(scene);
            VADON_ASSERT(scene_context_it != m_scene_canvas_contexts.end(), "Cannot find scene context!");
            init_view_entity(ecs_world, entity, scene_context_it->second.layers.front());
        }

        void init_view_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity, Vadon::Render::Canvas::LayerHandle scene_layer)
        {
            Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
            VadonDemo::View::ViewComponent* view_component = component_manager.get_component<VadonDemo::View::ViewComponent>(entity);
            if (view_component != nullptr)
            {
                // Make sure the resource is up-to-date
                if (view_component->resource.is_valid() == true)
                {
                    init_view_resource(view_component->resource);
                }

                if (view_component->canvas_item.is_valid() == false)
                {
                    Vadon::Render::Canvas::CanvasSystem& canvas_system = get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();
                    view_component->canvas_item = canvas_system.create_item(Vadon::Render::Canvas::ItemInfo{ .layer = scene_layer });

                    // Update view
                    m_view->update_view_entity_draw_data(ecs_world, entity);
                }
            }

            Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
            const Vadon::ECS::EntityList child_entities = entity_manager.get_children(entity);
            for (Vadon::ECS::EntityHandle current_child_entity : child_entities)
            {
                init_view_entity(ecs_world, current_child_entity, scene_layer);
            }
        }

        void remove_view_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
        {
            Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
            VadonDemo::View::ViewComponent* view_component = component_manager.get_component<VadonDemo::View::ViewComponent>(entity);
            
            if (view_component == nullptr)
            {
                return;
            }

            if (view_component->canvas_item.is_valid() == false)
            {
                return;
            }

            Vadon::Render::Canvas::CanvasSystem& canvas_system = get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();
            canvas_system.remove_item(view_component->canvas_item);
            view_component->canvas_item.invalidate();
        }

        void init_ui_entity(Vadon::ECS::EntityHandle entity)
        {
            VadonEditor::Model::Scene* scene = find_entity_scene(entity);
            if (scene == nullptr)
            {
                // Scene not available yet, add to deferred queue
                m_deferred_entity_init_queue.push_back(entity);
                return;
            }

            // Scene has layer, add entity
            VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
            Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

            auto scene_context_it = m_scene_canvas_contexts.find(scene);
            VADON_ASSERT(scene_context_it != m_scene_canvas_contexts.end(), "Cannot find scene context!");
            init_ui_entity(ecs_world, entity, scene_context_it->second.layers.back());
        }

        void init_ui_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity, Vadon::Render::Canvas::LayerHandle scene_layer)
        {
            Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
            VadonDemo::UI::Base* base_component = component_manager.get_component<VadonDemo::UI::Base>(entity);
            if (base_component != nullptr)
            {
                if (base_component->canvas_item.is_valid() == false)
                {
                    Vadon::Render::Canvas::CanvasSystem& canvas_system = get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();
                    base_component->canvas_item = canvas_system.create_item(Vadon::Render::Canvas::ItemInfo{ .layer = scene_layer });

                    // Update UI
                    m_ui->update_ui_element(ecs_world, entity);
                }
            }

            Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
            const Vadon::ECS::EntityList child_entities = entity_manager.get_children(entity);
            for (Vadon::ECS::EntityHandle current_child_entity : child_entities)
            {
                init_ui_entity(ecs_world, current_child_entity, scene_layer);
            }
        }

        void update_ui_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
        {
            Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

            VadonDemo::UI::Base* base_component = component_manager.get_component<VadonDemo::UI::Base>(entity);
            if (base_component == nullptr)
            {
                return;
            }

            if (base_component->canvas_item.is_valid() == false)
            {
                return;
            }

            m_ui->update_ui_element(ecs_world, entity);
        }

        void remove_ui_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
        {
            Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
            VadonDemo::UI::Base* base_component = component_manager.get_component<VadonDemo::UI::Base>(entity);
            if (base_component != nullptr)
            {
                m_ui->remove_ui_element(ecs_world, entity);
            }
        }

        void init_view_resource(VadonDemo::View::ViewResourceHandle resource_handle)
        {
            Vadon::Scene::ResourceSystem& resource_system = get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
            const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);

            if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::View::Sprite>(), resource_info.type_id))
            {
                // Check if referenced sprite is already loaded
                VadonDemo::View::SpriteResourceHandle sprite_handle = VadonDemo::View::SpriteResourceHandle::from_resource_handle(resource_handle);

                VadonDemo::View::Sprite* sprite_resource = resource_system.get_resource(sprite_handle);

                auto sprite_it = m_sprite_textures.find(sprite_resource->texture_path);
                if (sprite_it == m_sprite_textures.end())
                {
                    // Sprite not yet loaded
                    update_sprite_resource(sprite_handle);
                }
            }
        }

        void update_view_resource(VadonDemo::View::ViewResourceHandle resource_handle)
        {
            Vadon::Scene::ResourceSystem& resource_system = get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
            const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);

            if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::View::Sprite>(), resource_info.type_id))
            {
                update_sprite_resource(VadonDemo::View::SpriteResourceHandle::from_resource_handle(resource_handle));
            }
        }

        void update_sprite_resource(VadonDemo::View::SpriteResourceHandle sprite_handle)
        {
            load_sprite_resource(sprite_handle);

            VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
            m_view->update_view_resource(editor_model.get_ecs_world(), VadonDemo::View::ViewResourceHandle::from_resource_handle(sprite_handle));
        }

        void load_sprite_resource(VadonDemo::View::SpriteResourceHandle sprite_handle)
        {
            Vadon::Scene::ResourceSystem& resource_system = get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
            VadonDemo::View::Sprite* sprite_resource = resource_system.get_resource(sprite_handle);

            if (sprite_resource->texture_path.empty())
            {
                sprite_resource->texture_srv.invalidate();
                return;
            }

            auto sprite_it = m_sprite_textures.find(sprite_resource->texture_path);
            if (sprite_it != m_sprite_textures.end())
            {
                sprite_resource->texture_srv = sprite_it->second.srv;
                return;
            }

            // FIXME: accept other extensions!
            std::filesystem::path texture_fs_path = sprite_resource->texture_path;
            texture_fs_path.replace_extension(".dds");

            VadonEditor::Core::ProjectManager& project_manager = get_system<VadonEditor::Core::ProjectManager>();
            Vadon::Core::FileSystem& file_system = get_engine_core().get_system<Vadon::Core::FileSystem>();
            Vadon::Core::FileSystemPath file_path{ .root_directory = project_manager.get_active_project().root_dir_handle, .path = texture_fs_path.string() };
            
            if (file_system.does_file_exist(file_path) == false)
            {
                sprite_resource->texture_srv.invalidate();
                return;
            }

            Vadon::Core::FileSystem::RawFileDataBuffer texture_file_buffer;
            if (file_system.load_file(file_path, texture_file_buffer) == false)
            {
                sprite_resource->texture_srv.invalidate();
                return;
            }

            Vadon::Render::TextureSystem& texture_system = get_engine_core().get_system<Vadon::Render::TextureSystem>();
            Vadon::Render::TextureHandle texture_handle = texture_system.create_texture_from_memory(texture_file_buffer.size(), texture_file_buffer.data());
            if (texture_handle.is_valid() == false)
            {
                sprite_resource->texture_srv.invalidate();
                return;
            }

            const Vadon::Render::TextureInfo texture_info = texture_system.get_texture_info(texture_handle);

            // Create texture view
            // FIXME: we are guessing the 
            Vadon::Render::TextureSRVInfo texture_srv_info;
            texture_srv_info.format = texture_info.format;
            texture_srv_info.type = Vadon::Render::TextureSRVType::TEXTURE_2D;
            texture_srv_info.mip_levels = texture_info.mip_levels;
            texture_srv_info.most_detailed_mip = 0;

            Vadon::Render::SRVHandle srv_handle = texture_system.create_shader_resource_view(texture_handle, texture_srv_info);
            if (srv_handle.is_valid() == false)
            {
                // TODO: also remove texture?
                sprite_resource->texture_srv.invalidate();
                return;
            }

            // Add to lookup
            SpriteTexture new_sprite_texture{ .texture = texture_handle, .srv = srv_handle };
            m_sprite_textures.insert(std::make_pair(sprite_resource->texture_path, new_sprite_texture));

            sprite_resource->texture_srv = srv_handle;
        }

        enum class InputAction
        {
            CAMERA_UP,
            CAMERA_DOWN,
            CAMERA_LEFT,
            CAMERA_RIGHT,
            CAMERA_ZOOM,
            ACTION_COUNT
        };

        std::array<VadonApp::Platform::InputActionHandle, Vadon::Utilities::to_integral(InputAction::ACTION_COUNT)> m_input_actions;

        std::unique_ptr<VadonDemo::Model::Model> m_model;
        std::unique_ptr<VadonDemo::UI::UI> m_ui;
        std::unique_ptr<VadonDemo::View::View> m_view;
       
        std::unordered_map<VadonEditor::Model::Scene*, Vadon::Render::Canvas::RenderContext> m_scene_canvas_contexts;
        std::vector<Vadon::ECS::EntityHandle> m_deferred_entity_init_queue;

        // FIXME: refactor this, need a proper asset management system!
        struct SpriteTexture
        {
            Vadon::Render::TextureHandle texture;
            Vadon::Render::SRVHandle srv;
        };

        std::unordered_map<std::string, SpriteTexture> m_sprite_textures;
    };
}

int main(int argc, char* argv[])
{
    Vadon::Core::EngineEnvironment engine_environment;

    Editor editor(engine_environment);
    return editor.execute(argc, argv);
}