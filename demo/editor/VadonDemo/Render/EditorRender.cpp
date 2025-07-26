#include <VadonDemo/Render/EditorRender.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Core/Editor.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/Platform/PlatformInterface.hpp>
#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/View/ViewSystem.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>
#include <Vadon/Render/Frame/FrameSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

namespace VadonDemo::Render
{
    CanvasContextHandle EditorRender::get_scene_canvas_context(const VadonEditor::Model::Scene* scene)
    {
        VADON_ASSERT(scene != nullptr, "Scene must not be null!");

        auto scene_context_it = m_scene_canvas_contexts.find(scene);
        if (scene_context_it == m_scene_canvas_contexts.end())
        {
            Render& common_render = m_editor.get_core().get_render();
            CanvasContextHandle new_context = common_render.create_canvas_context();

            // Scene not displayed before, set up new context
            Vadon::Render::Canvas::RenderContext& render_context = common_render.get_context(new_context);

            Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_common_editor().get_engine_core();

            // Set up viewport based on main window
            {
                Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();

                Vadon::Render::Canvas::Viewport canvas_viewport;
                canvas_viewport.render_target = rt_system.get_window_back_buffer_view(m_render_window);

                render_context.viewports.push_back(canvas_viewport);
            }

            scene_context_it = m_scene_canvas_contexts.insert(std::make_pair(scene, new_context)).first;
        }

        return scene_context_it->second;
    }

    void EditorRender::init_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_common_editor().get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();
        Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

        Render& common_render = m_editor.get_core().get_render();

        // TODO: implement better decoupling of the main system from the component-specific logic
        // Have shared code for managing certain resources, but otherwise delegate to subsystems
        // (e.g SpriteTiling could use shared texture logic, the component should just provide parameters)

        if(CanvasComponent* canvas_component = component_manager.get_component<CanvasComponent>(entity))
        {
            // If Canvas item is set, assume it's already initialized
            // FIXME: check if context is correctly set?
            if (canvas_component->canvas_item.is_valid() == false)
            {
                const VadonEditor::Model::Scene* entity_scene = m_editor.find_entity_scene(entity);
                VADON_ASSERT(entity_scene != nullptr, "Cannot find scene!");

                const CanvasContextHandle scene_context = get_scene_canvas_context(entity_scene);
                common_render.init_canvas_entity(ecs_world, entity, scene_context);

                // Enqueue an update for our layers (just in case this entity caused the layer to be added)
                // FIXME: revise so we can track this explicitly and only do it when needed!
                set_layers_dirty();
            }
        }

        if (SpriteTilingComponent* sprite_component = component_manager.get_component<SpriteTilingComponent>(entity))
        {
            load_texture_resource_data(sprite_component->texture);
            common_render.init_sprite_tiling_entity(ecs_world, entity);
        }

        if (FullscreenEffectComponent* fullscreen_effect_component = component_manager.get_component<FullscreenEffectComponent>(entity))
        {
            load_shader_resource_data(fullscreen_effect_component->shader);
            common_render.init_fullscreen_effect_entity(ecs_world, entity);
        }
    }

    void EditorRender::load_texture_resource_data(TextureResourceID texture_id)
    {
        if (texture_id.is_valid() == false)
        {
            return;
        }

        // Reload texture
        VadonDemo::Render::Render& common_render = m_editor.get_core().get_render();
        common_render.init_texture_resource(texture_id);
    }

    void EditorRender::load_shader_resource_data(ShaderResourceID shader_id)
    {
        if (shader_id.is_valid() == false)
        {
            return;
        }

        // Reload shader
        VadonDemo::Render::Render& common_render = m_editor.get_core().get_render();
        common_render.init_shader_resource(shader_id);
    }

    EditorRender::EditorRender(Core::Editor& editor)
        : m_editor(editor)
        , m_layers_dirty(false)
    { }

    bool EditorRender::initialize()
    {
        VadonEditor::Core::Editor& common_editor = m_editor.get_common_editor();
        VadonApp::Platform::WindowHandle main_window = common_editor.get_system<VadonEditor::Platform::PlatformInterface>().get_main_window();

        VadonApp::Core::Application& engine_app = common_editor.get_engine_app();
        VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();

        Vadon::Render::WindowInfo render_window_info;
        render_window_info.platform_handle = platform_interface.get_platform_window_handle(main_window);
        render_window_info.format = Vadon::Render::GraphicsAPIDataFormat::B8G8R8A8_UNORM;

        Vadon::Core::EngineCoreInterface& engine_core = engine_app.get_engine_core();

        Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
        m_render_window = rt_system.create_window(render_window_info);

        if (m_render_window.is_valid() == false)
        {
            return false;
        }

        init_frame_graph();

        return true;
    }
    bool EditorRender::init_frame_graph()
    {
        // FIXME: should not have a fixed frame graph, instead process active viewports and render tasks
        VadonEditor::Core::Editor& common_editor = m_editor.get_common_editor();
        VadonApp::Core::Application& engine_app = common_editor.get_engine_app();
        Vadon::Core::EngineCoreInterface& engine_core = common_editor.get_engine_core();

        // Draw to the main window
        // FIXME: draw to separate RT and copy to back buffer at the end!		
        Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
        const Vadon::Render::RTVHandle main_window_target = rt_system.get_window_back_buffer_view(m_render_window);

        // Create frame graph
        // FIXME: make this even more flexible, maybe even possible to set purely from data (so model doesn't even reference systems explicitly)
        Vadon::Render::FrameGraphInfo frame_graph_info;
        frame_graph_info.targets.emplace_back("main_window");

        // Start with a pass that clears everything and sets shared state
        constexpr const char* clear_pass_target = "main_window_cleared";
        {
            Vadon::Render::RenderPass& clear_pass = frame_graph_info.passes.emplace_back();
            clear_pass.name = "Clear";

            clear_pass.targets.emplace_back("main_window", clear_pass_target);
            clear_pass.execution = [main_window_target, &rt_system]()
                {
                    rt_system.clear_target(main_window_target, Vadon::Render::RGBAColor(0.0f, 0.0f, 0.0f, 1.0f));
                    rt_system.set_target(main_window_target, Vadon::Render::DSVHandle());
                };
        }

        // Canvas
        constexpr const char* canvas_pass_target = "main_window_canvas";
        {
            Vadon::Render::RenderPass& canvas_pass = frame_graph_info.passes.emplace_back();
            canvas_pass.name = "Canvas";

            canvas_pass.targets.emplace_back(clear_pass_target, canvas_pass_target);

            canvas_pass.execution = [this]()
                {
                    VadonEditor::Core::Editor& common_editor = m_editor.get_common_editor();
                    VadonEditor::View::ViewModel& view_model = common_editor.get_system<VadonEditor::View::ViewSystem>().get_view_model();
                    VadonEditor::Model::Scene* active_scene = view_model.get_active_scene();

                    if (active_scene == nullptr)
                    {
                        // Nothing to render
                        return;
                    }

                    if (m_layers_dirty == true)
                    {
                        update_dirty_layers();
                    }

                    CanvasContextHandle active_context = get_scene_canvas_context(active_scene);
                    Vadon::Render::Canvas::RenderContext render_context;
                    Render& common_render = m_editor.get_core().get_render();
                    {

                        const Vadon::Render::Canvas::RenderContext& active_render_context = common_render.get_context(active_context);
                        render_context = active_render_context;
                    }

                    // Update viewport and camera rectangle
                    // FIXME: only update this when it changes!
                    {
                        VadonApp::Platform::WindowHandle main_window = common_editor.get_system<VadonEditor::Platform::PlatformInterface>().get_main_window();

                        VadonApp::Platform::PlatformInterface& platform_interface = common_editor.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
                        const Vadon::Utilities::Vector2i window_size = platform_interface.get_window_drawable_size(main_window);

                        render_context.camera.view_rectangle.size = window_size;
                        render_context.viewports.back().render_viewport.dimensions.size = window_size;

                        render_context.layers.push_back(m_editor_layer);
                    }

                    // Update sprite tiling (needs the camera view rectangle)
                    {
                        Vadon::ECS::World& ecs_world = common_editor.get_system<VadonEditor::Model::ModelSystem>().get_ecs_world();
                        auto sprite_query = ecs_world.get_component_manager().run_component_query<CanvasComponent&, SpriteTilingComponent&>();

                        Vadon::Render::Rectangle culling_rect = render_context.camera.view_rectangle;
                        culling_rect.size /= render_context.camera.zoom;

                        for (auto sprite_it = sprite_query.get_iterator(); sprite_it.is_valid() == true; sprite_it.next())
                        {
                            auto sprite_tuple = sprite_it.get_tuple();
                            SpriteTilingComponent& current_sprite_component = std::get<SpriteTilingComponent&>(sprite_tuple);
                            const CanvasComponent& current_canvas_component = std::get<CanvasComponent&>(sprite_tuple);

                            common_render.update_sprite_tiling_entity(current_canvas_component, current_sprite_component, culling_rect);
                        }
                    }

                    Vadon::Render::Canvas::CanvasSystem& canvas_system = common_editor.get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();
                    canvas_system.render(render_context);
                };
        }

        // Dev GUI
        constexpr const char* dev_gui_target = "dev_gui";
        {
            Vadon::Render::RenderPass& dev_gui_pass = frame_graph_info.passes.emplace_back();

            dev_gui_pass.name = "DevGUI";
            dev_gui_pass.targets.emplace_back(canvas_pass_target, dev_gui_target); // FIXME: make sure these pass-related names are accessible so we're not implicitly using the same string!

            VadonEditor::UI::UISystem& ui_system = common_editor.get_system<VadonEditor::UI::UISystem>();
            VadonApp::UI::Developer::GUISystem& dev_gui_system = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();
            dev_gui_pass.execution = [this, &ui_system, &dev_gui_system]()
                {
                    dev_gui_system.render();
                };
        }

        Vadon::Render::FrameSystem& frame_system = common_editor.get_engine_core().get_system<Vadon::Render::FrameSystem>();
        m_frame_graph = frame_system.create_graph(frame_graph_info);

        if (m_frame_graph.is_valid() == false)
        {
            return false;
        }

        return true;
    }
    
    bool EditorRender::project_loaded()
    {
        VadonEditor::Core::Editor& common_editor = m_editor.get_common_editor();
        VadonEditor::Model::ModelSystem& editor_model = common_editor.get_system<VadonEditor::Model::ModelSystem>();
        VadonEditor::Model::SceneSystem& editor_scene_system = editor_model.get_scene_system();

        editor_scene_system.add_entity_event_callback(
            [this](const VadonEditor::Model::EntityEvent& event)
            {
                switch (event.type)
                {
                case VadonEditor::Model::EntityEventType::ADDED:
                    init_entity(event.entity);
                    break;
                case VadonEditor::Model::EntityEventType::REMOVED:
                    remove_entity(event.entity);
                    break;
                }
            }
        );

        editor_scene_system.add_component_event_callback(
            [this](const VadonEditor::Model::ComponentEvent& event)
            {
                if (event.component_type != Vadon::Utilities::TypeRegistry::get_type_id<CanvasComponent>()
                    || event.component_type != Vadon::Utilities::TypeRegistry::get_type_id<SpriteTilingComponent>()
                    || event.component_type != Vadon::Utilities::TypeRegistry::get_type_id<FullscreenEffectComponent>())
                {
                    return;
                }

                switch (event.type)
                {
                case VadonEditor::Model::ComponentEventType::ADDED:
                    init_entity(event.owner);
                    break;
                case VadonEditor::Model::ComponentEventType::EDITED:
                    update_entity(event.owner);
                    break;
                case VadonEditor::Model::ComponentEventType::REMOVED:
                    remove_entity(event.owner);
                    break;
                }
            }
        );

        VadonEditor::Model::ResourceSystem& editor_resource_system = editor_model.get_resource_system();
        editor_resource_system.register_edit_callback(
            [this, &editor_model](Vadon::Scene::ResourceID resource_id)
            {
                Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_common_editor().get_engine_core();
                Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
                Vadon::Scene::ResourceHandle resource_handle = resource_system.find_resource(resource_id);
                VADON_ASSERT(resource_handle.is_valid() == true, "Resource not found!");

                const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
                if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<CanvasLayerDefinition>(), resource_info.type_id))
                {
                    VadonDemo::Render::Render& common_render = m_editor.get_core().get_render();
                    common_render.update_layer_definition(CanvasLayerDefHandle::from_resource_handle(resource_handle));

                    set_layers_dirty();
                }
                else if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<TextureResource>())
                {
                    const TextureResourceID texture_id = TextureResourceID::from_resource_id(resource_info.id);

                    // First unload the old texture
                    VadonDemo::Render::Render& common_render = m_editor.get_core().get_render();
                    common_render.unload_texture_resource(texture_id);

                    Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

                    Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();
                    auto sprite_query = ecs_world.get_component_manager().run_component_query<CanvasComponent&, SpriteTilingComponent&>();
                    
                    for (auto sprite_it = sprite_query.get_iterator(); sprite_it.is_valid() == true; sprite_it.next())
                    {
                        auto sprite_tuple = sprite_it.get_tuple();
                        SpriteTilingComponent& current_sprite_component = std::get<SpriteTilingComponent&>(sprite_tuple);

                        if (current_sprite_component.texture != texture_id)
                        {
                            continue;
                        }

                        // Make sure texture is loaded
                        load_texture_resource_data(texture_id);

                        // Clear the canvas item
                        const CanvasComponent& current_canvas_component = std::get<CanvasComponent&>(sprite_tuple);
                        canvas_system.clear_item(current_canvas_component.canvas_item);

                        // Reset rect (this will force an update)
                        current_sprite_component.reset_rect();
                    }
                }
                else if (resource_info.type_id == Vadon::Utilities::TypeRegistry::get_type_id<ShaderResource>())
                {
                    const ShaderResourceID shader_id = ShaderResourceID::from_resource_id(resource_info.id);

                    // First unload the old shader
                    VadonDemo::Render::Render& common_render = m_editor.get_core().get_render();
                    common_render.unload_shader_resource(shader_id);

                    // Then re-load using new params
                    load_shader_resource_data(shader_id);
                }
            }
        );

        // Create editor layer and item (e.g to show viewport rectangle)
        {
            Vadon::Render::Canvas::CanvasSystem& canvas_system = common_editor.get_engine_core().get_system<Vadon::Render::Canvas::CanvasSystem>();

            m_editor_layer = canvas_system.create_layer(Vadon::Render::Canvas::LayerInfo{});

            m_editor_item = canvas_system.create_item(Vadon::Render::Canvas::ItemInfo{ .layer = m_editor_layer });
            update_editor_layer();
        }

        return true;
    }

    void EditorRender::update()
    {
        process_platform_events();

        // Execute the frame graph
        VadonEditor::Core::Editor& common_editor = m_editor.get_common_editor();
        Vadon::Core::EngineCoreInterface& engine_core = common_editor.get_engine_core();

        Vadon::Render::FrameSystem& frame_system = engine_core.get_system<Vadon::Render::FrameSystem>();
        frame_system.execute_graph(m_frame_graph);

        // Present to the main window
        Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();
        rt_system.update_window(Vadon::Render::WindowUpdateInfo{ .window = m_render_window });
    }

    void EditorRender::update_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_common_editor().get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();
        Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

        if (CanvasComponent* canvas_component = component_manager.get_component<CanvasComponent>(entity))
        {
            m_editor.get_core().get_render().update_canvas_entity(ecs_world, entity);
            set_layers_dirty();
        }

        if (SpriteTilingComponent* sprite_component = component_manager.get_component<SpriteTilingComponent>(entity))
        {
            if (sprite_component->texture.is_valid() == true)
            {
                load_texture_resource_data(sprite_component->texture);
            }

            // Reset rect to make sure we redraw next frame
            sprite_component->reset_rect();
        }

        if (FullscreenEffectComponent* fullscreen_effect_component = component_manager.get_component<FullscreenEffectComponent>(entity))
        {
            
            m_editor.get_core().get_render().update_fullscreen_effect_entity(ecs_world, entity);
        }
    }

    void EditorRender::remove_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_common_editor().get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();
        Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

        if (CanvasComponent* canvas_component = component_manager.get_component<CanvasComponent>(entity))
        {
            m_editor.get_core().get_render().remove_canvas_entity(ecs_world, entity);
        }

        if (SpriteTilingComponent* sprite_component = component_manager.get_component<SpriteTilingComponent>(entity))
        {
            m_editor.get_core().get_render().remove_sprite_tiling_entity(ecs_world, entity);
        }

        if (FullscreenEffectComponent* fullscreen_effect_component = component_manager.get_component<FullscreenEffectComponent>(entity))
        {
            m_editor.get_core().get_render().remove_fullscreen_effect_entity(ecs_world, entity);
        }
    }

    void EditorRender::update_background_sprite_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_common_editor().get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        SpriteTilingComponent* sprite_component = ecs_world.get_component_manager().get_component<SpriteTilingComponent>(entity);
        VADON_ASSERT(sprite_component, "Missing component!");
        
        // Make the rect empty so we're forced to recalculate next frame
        sprite_component->reset_rect();
    }

    void EditorRender::update_fullscreen_effect_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_common_editor().get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        m_editor.get_core().get_render().update_fullscreen_effect_entity(ecs_world, entity);
    }

    void EditorRender::update_dirty_layers()
    {
        // The editor needs to display the layers in a specific way, so after using the common code path, we "fix up" for the editor afterward
        m_layers_dirty = false;

        Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_common_editor().get_engine_core();
        Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
        VadonDemo::Render::Render& common_render = m_editor.get_core().get_render();

        for (auto context_pair : m_scene_canvas_contexts)
        {
            Vadon::Render::Canvas::RenderContext& current_context = common_render.get_context(context_pair.second);
            for (Vadon::Render::Canvas::LayerHandle current_layer_handle : current_context.layers)
            {
                const Vadon::Render::Canvas::LayerInfo current_layer_info = canvas_system.get_layer_info(current_layer_handle);

                // NOTE: filtering out "view agnostic" flag, in-editor we want to see how things are positioned w.r.t the viewport
                const auto flags_integral = Vadon::Utilities::to_integral(current_layer_info.flags) & ~Vadon::Utilities::to_integral(Vadon::Render::Canvas::LayerFlags::VIEW_AGNOSTIC);

                canvas_system.set_layer_flags(current_layer_handle, Vadon::Utilities::to_enum<Vadon::Render::Canvas::LayerFlags>(flags_integral));
            }
        }
    }

    void EditorRender::update_editor_layer()
    {
        Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_common_editor().get_engine_core();
        Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

        canvas_system.clear_item(m_editor_item);

        // Add viewport
        {
            const Vadon::Utilities::Vector2& viewport_size = m_editor.get_core().get_global_config().viewport_size;

            Vadon::Render::Canvas::Rectangle viewport_rectangle;
            viewport_rectangle.color = Vadon::Render::ColorRGBA::from_rgba_vector(Vadon::Utilities::ColorVector{ 0.0f, 0.0f, 1.0f, 1.0f });
            viewport_rectangle.dimensions.size = viewport_size;
            viewport_rectangle.thickness = 1.0f;
            viewport_rectangle.filled = false;

            canvas_system.draw_item_rectangle(m_editor_item, viewport_rectangle);
        }
    }

    void EditorRender::process_platform_events()
    {
        // Add event handler for window move & resize (affects rendering so it has to happen at the appropriate time)
        VadonEditor::Core::Editor& common_editor = m_editor.get_common_editor();
        VadonApp::Core::Application& engine_app = common_editor.get_engine_app();
        VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();

        for (const VadonApp::Platform::PlatformEvent& current_event : platform_interface.poll_events())
        {
            const VadonApp::Platform::PlatformEventType current_event_type = Vadon::Utilities::to_enum<VadonApp::Platform::PlatformEventType>(static_cast<int32_t>(current_event.index()));
            switch (current_event_type)
            {
            case VadonApp::Platform::PlatformEventType::WINDOW:
            {
                const VadonApp::Platform::WindowEvent& window_event = std::get<VadonApp::Platform::WindowEvent>(current_event);
                switch (window_event.type)
                {
                case VadonApp::Platform::WindowEventType::RESIZED:
                {
                    // Get drawable size															
                    VadonApp::Platform::WindowHandle main_window = common_editor.get_system<VadonEditor::Platform::PlatformInterface>().get_main_window();
                    const Vadon::Utilities::Vector2i drawable_size = platform_interface.get_window_drawable_size(main_window);

                    // Resize the render window
                    Vadon::Render::RenderTargetSystem& rt_system = engine_app.get_engine_core().get_system<Vadon::Render::RenderTargetSystem>();
                    rt_system.resize_window(m_render_window, drawable_size);
                }
                break;
                }
            }
            break;
            }
        }
    }
}