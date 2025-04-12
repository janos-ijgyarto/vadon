#include <VadonDemo/Render/EditorRender.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Core/Editor.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/Platform/PlatformInterface.hpp>
#include <VadonEditor/Render/RenderSystem.hpp>
#include <VadonEditor/View/ViewSystem.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>

#include <Vadon/Core/File/Path.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>
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

            Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();

            // Set up viewport based on main window
            {
                Vadon::Render::RenderTargetSystem& rt_system = engine_core.get_system<Vadon::Render::RenderTargetSystem>();

                Vadon::Render::Canvas::Viewport canvas_viewport;
                canvas_viewport.render_target = rt_system.get_window_back_buffer_view(m_editor.get_system<VadonEditor::Render::RenderSystem>().get_render_window());

                render_context.viewports.push_back(canvas_viewport);
            }

            scene_context_it = m_scene_canvas_contexts.insert(std::make_pair(scene, new_context)).first;
        }

        return scene_context_it->second;
    }

    TextureResource* EditorRender::get_texture_resource(std::string_view path)
    {
        const std::string path_string(path);
        auto texture_it = m_textures.find(path_string);
        if (texture_it == m_textures.end())
        {
            VadonEditor::Core::ProjectManager& project_manager = m_editor.get_system<VadonEditor::Core::ProjectManager>();
            TextureResource new_resource = m_editor.get_core().get_render().load_texture_resource(Vadon::Core::FileSystemPath{ .root_directory = project_manager.get_active_project().root_dir_handle, .path = std::string(path) });

            if (new_resource.texture.is_valid() == false)
            {
                return nullptr;
            }

            // Add to lookup
            texture_it = m_textures.insert(std::make_pair(path_string, new_resource)).first;
        }

        return &texture_it->second;
    }

    void EditorRender::init_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(entity);
        if (canvas_component == nullptr)
        {
            return;
        }

        if (canvas_component->canvas_item.is_valid() == true)
        {
            // Canvas item already initialized
            // FIXME: check if context is correctly set?
            return;
        }

        const VadonEditor::Model::Scene* entity_scene = m_editor.find_entity_scene(entity);
        VADON_ASSERT(entity_scene != nullptr, "Cannot find scene!");

        const CanvasContextHandle scene_context = get_scene_canvas_context(entity_scene);
        m_editor.get_core().get_render().init_entity(ecs_world, entity, scene_context);
        set_layers_dirty();
    }

    EditorRender::EditorRender(Core::Editor& editor)
        : m_editor(editor)
        , m_layers_dirty(false)
    { }

    bool EditorRender::initialize()
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
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
                if (event.component_type != Vadon::Utilities::TypeRegistry::get_type_id<CanvasComponent>())
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
            [this](Vadon::Scene::ResourceID resource_id)
            {
                Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
                Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
                Vadon::Scene::ResourceHandle resource_handle = resource_system.find_resource(resource_id);
                VADON_ASSERT(resource_handle.is_valid() == true, "Resource not found!");

                const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
                if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::Render::CanvasLayerDefinition>(), resource_info.type_id))
                {
                    VadonDemo::Render::Render& common_render = m_editor.get_core().get_render();
                    common_render.update_layer_definition(CanvasLayerDefHandle::from_resource_handle(resource_handle));

                    set_layers_dirty();
                }
            }
        );

        // Create editor layer and item (e.g to show viewport rectangle)
        {
            Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
            Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

            m_editor_layer = canvas_system.create_layer(Vadon::Render::Canvas::LayerInfo{});

            m_editor_item = canvas_system.create_item(Vadon::Render::Canvas::ItemInfo{ .layer = m_editor_layer });
            update_editor_layer();
        }

        return true;
    }

    void EditorRender::update()
    {
        VadonEditor::View::ViewModel& view_model = m_editor.get_system<VadonEditor::View::ViewSystem>().get_view_model();
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

        Render& common_render = m_editor.get_core().get_render();
        const Vadon::Render::Canvas::RenderContext& active_render_context = common_render.get_context(active_context);
        Vadon::Render::Canvas::RenderContext render_context = active_render_context;

        // Update viewport and camera rectangle
        // FIXME: only update this when it changes!
        VadonApp::Core::Application& engine_app = m_editor.get_engine_app();

        VadonApp::Platform::WindowHandle main_window = m_editor.get_system<VadonEditor::Platform::PlatformInterface>().get_main_window();

        VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
        const Vadon::Utilities::Vector2i window_size = platform_interface.get_window_drawable_size(main_window);
        render_context.camera.view_rectangle.size = window_size;
        render_context.viewports.back().render_viewport.dimensions.size = window_size;

        render_context.layers.push_back(m_editor_layer);

        VadonEditor::Render::RenderSystem& editor_render = m_editor.get_system<VadonEditor::Render::RenderSystem>();
        editor_render.enqueue_canvas(render_context);
    }

    void EditorRender::update_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        m_editor.get_core().get_render().update_entity(ecs_world, entity);
        set_layers_dirty();
    }

    void EditorRender::remove_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        m_editor.get_core().get_render().remove_entity(ecs_world, entity);
    }

    void EditorRender::update_dirty_layers()
    {
        // The editor needs to display the layers in a specific way, so after using the common code path, we "fix up" for the editor afterward
        m_layers_dirty = false;

        Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
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
        Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
        Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

        canvas_system.clear_item(m_editor_item);

        // Add viewport
        {
            const Vadon::Utilities::Vector2& viewport_size = m_editor.get_core().get_global_config().viewport_size;

            Vadon::Render::Canvas::Rectangle viewport_rectangle;
            viewport_rectangle.color = Vadon::Render::Canvas::ColorRGBA{ 0.0f, 0.0f, 1.0f, 1.0f };
            viewport_rectangle.dimensions.size = viewport_size;
            viewport_rectangle.thickness = 1.0f;
            viewport_rectangle.filled = false;

            canvas_system.draw_item_rectangle(m_editor_item, viewport_rectangle);
        }
    }
}