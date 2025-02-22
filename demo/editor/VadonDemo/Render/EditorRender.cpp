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
                canvas_viewport.render_target = rt_system.get_window_target(m_editor.get_system<VadonEditor::Render::RenderSystem>().get_render_window());

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

    EditorRender::EditorRender(Core::Editor& editor)
        : m_editor(editor)
    { }

    bool EditorRender::initialize()
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();
        Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

        component_manager.register_event_callback<VadonDemo::Render::CanvasComponent>(
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

        VadonEditor::Model::SceneSystem& editor_scene_system = editor_model.get_scene_system();
        editor_scene_system.register_edit_callback(
            [this](Vadon::ECS::EntityHandle entity, Vadon::Utilities::TypeID component_type)
            {
                if (component_type == Vadon::Utilities::TypeRegistry::get_type_id<CanvasComponent>())
                {
                    update_entity(entity);
                }
            }
        );

        VadonEditor::Model::ResourceSystem& editor_resource_system = editor_model.get_resource_system();
        editor_resource_system.register_edit_callback(
            [this](Vadon::Scene::ResourceID resource_id)
            {
                Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
                Vadon::Scene::ResourceHandle resource_handle = resource_system.find_resource(resource_id);
                VADON_ASSERT(resource_handle.is_valid() == true, "Resource not found!");

                const Vadon::Scene::ResourceInfo resource_info = resource_system.get_resource_info(resource_handle);
                if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::Render::CanvasLayerDefinition>(), resource_info.type_id))
                {
                    m_editor.get_core().get_render().update_layer_definition(CanvasLayerDefHandle::from_resource_handle(resource_handle));
                }
            }
        );

        return true;
    }

    void EditorRender::update()
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
            // Nothing to render
            return;
        }

        CanvasContextHandle active_context = get_scene_canvas_context(active_scene);

        Render& common_render = m_editor.get_core().get_render();
        Vadon::Render::Canvas::RenderContext& render_context = common_render.get_context(active_context);

        // Update viewport and camera rectangle
        // FIXME: only update this when it changes!
        VadonApp::Core::Application& engine_app = m_editor.get_engine_app();

        VadonApp::Platform::WindowHandle main_window = m_editor.get_system<VadonEditor::Platform::PlatformInterface>().get_main_window();

        VadonApp::Platform::PlatformInterface& platform_interface = engine_app.get_system<VadonApp::Platform::PlatformInterface>();
        const Vadon::Utilities::Vector2i window_size = platform_interface.get_window_drawable_size(main_window);
        render_context.camera.view_rectangle.size = window_size;
        render_context.viewports.back().render_viewport.dimensions.size = window_size;

        VadonEditor::Render::RenderSystem& editor_render = m_editor.get_system<VadonEditor::Render::RenderSystem>();
        editor_render.enqueue_canvas(render_context);
    }

    void EditorRender::update_entity(Vadon::ECS::EntityHandle entity)
    {
        auto entity_it = std::find(m_deferred_update_queue.begin(), m_deferred_update_queue.end(), entity);
        if (entity_it == m_deferred_update_queue.end())
        {
            m_deferred_update_queue.push_back(entity);
        }
    }

    void EditorRender::deferred_update_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        const VadonEditor::Model::Scene* entity_scene = m_editor.find_entity_scene(entity);
        VADON_ASSERT(entity_scene != nullptr, "Cannot find scene!");
        CanvasContextHandle scene_context = get_scene_canvas_context(entity_scene);

        m_editor.get_core().get_render().update_entity(ecs_world, entity, scene_context);
    }

    void EditorRender::remove_entity(Vadon::ECS::EntityHandle entity)
    {
        VadonEditor::Model::ModelSystem& editor_model = m_editor.get_system<VadonEditor::Model::ModelSystem>();
        Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

        m_editor.get_core().get_render().remove_entity(ecs_world, entity);
    }
}