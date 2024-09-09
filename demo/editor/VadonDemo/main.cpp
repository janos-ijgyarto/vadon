#include <VadonDemo/Model/Model.hpp>

#include <VadonDemo/Model/Component.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Render/RenderSystem.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>

#include <Vadon/Core/Environment.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/Context.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

namespace
{
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

            if (init_renderer() == false)
            {
                return false;
            }

            if (init_ecs() == false)
            {
                return false;
            }

            return true;
        }
    private:
        bool init_renderer()
        {
            const VadonApp::Platform::RenderWindowInfo main_window_info = get_engine_app().get_system<VadonApp::Platform::PlatformInterface>().get_window_info();

            m_canvas_context.camera.view_rectangle.size = main_window_info.window.size;
            m_canvas_context.layers.push_back(m_model->get_canvas_layer());

            {
                Vadon::Render::RenderTargetSystem& rt_system = get_engine_core().get_system<Vadon::Render::RenderTargetSystem>();

                Vadon::Render::Canvas::Viewport canvas_viewport;
                canvas_viewport.render_target = rt_system.get_window_target(main_window_info.render_handle);
                canvas_viewport.render_viewport.dimensions.size = main_window_info.window.size;

                m_canvas_context.viewports.push_back(canvas_viewport);
            }

            VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
            VadonEditor::Render::RenderSystem& editor_render = get_system<VadonEditor::Render::RenderSystem>();

            Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

            editor_model.add_callback(
                [this, &editor_model, &editor_render, &ecs_world]()
                {
                    m_model->update_rendering(ecs_world);
                    editor_render.enqueue_canvas(m_canvas_context);
                }
            );

            return true;
        }

        bool init_ecs()
        {
            VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
            Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

            Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
            component_manager.register_event_callback<VadonDemo::Model::CanvasComponent>(
                [this, &ecs_world](const Vadon::ECS::ComponentEvent& event)
                {
                    m_model->component_event(ecs_world, event);
                }
            );
            component_manager.register_event_callback<VadonDemo::Model::Celestial>(
                [this, &ecs_world](const Vadon::ECS::ComponentEvent& event)
                {
                    m_model->component_event(ecs_world, event);
                }
            );

            return true;
        }

        std::unique_ptr<VadonDemo::Model::Model> m_model;
        Vadon::Render::Canvas::RenderContext m_canvas_context;
    };
}

int main(int argc, char* argv[])
{
    Vadon::Core::EngineEnvironment engine_environment;

    Editor editor(engine_environment);
    return editor.execute(argc, argv);
}