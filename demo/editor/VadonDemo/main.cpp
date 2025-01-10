#include <VadonDemo/Model/Model.hpp>
#include <VadonDemo/Model/Component.hpp>

#include <VadonDemo/View/View.hpp>
#include <VadonDemo/View/Component.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Platform/PlatformInterface.hpp>
#include <VadonEditor/Render/RenderSystem.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/Platform/Input/InputSystem.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Core/Environment.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/Context.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>

#include <Vadon/Utilities/Container/Queue/PacketQueue.hpp>

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
            m_canvas_context.layers.push_back(m_view->get_canvas_layer());

            {
                Vadon::Render::RenderTargetSystem& rt_system = get_engine_core().get_system<Vadon::Render::RenderTargetSystem>();

                Vadon::Render::Canvas::Viewport canvas_viewport;
                canvas_viewport.render_target = rt_system.get_window_target(get_system<VadonEditor::Render::RenderSystem>().get_render_window());

                m_canvas_context.viewports.push_back(canvas_viewport);
            }

            VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
            VadonEditor::Render::RenderSystem& editor_render = get_system<VadonEditor::Render::RenderSystem>();

            Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

            editor_model.add_callback(
                [this, &editor_model, &editor_render, &ecs_world]()
                {
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
                    m_canvas_context.viewports.back().render_viewport.dimensions.size = window_size;

                    m_canvas_context.camera.view_rectangle.position += get_delta_time() * 200 * camera_velocity;
                    m_canvas_context.camera.view_rectangle.size = window_size;
                    m_canvas_context.camera.zoom = std::clamp(m_canvas_context.camera.zoom + get_delta_time() * camera_zoom * 10.0f, 0.1f, 10.0f);

                    // Continuously extract model state and render
                    // FIXME: make this event-driven so we only update when something changes?
                    m_view->extract_model_state(ecs_world);
                    m_view->update(ecs_world, 1.0f);

                    editor_render.enqueue_canvas(m_canvas_context);
                }
            );

            return true;
        }

        bool init_model_view()
        {
            VadonEditor::Model::ResourceSystem& resource_system = get_system<VadonEditor::Model::ModelSystem>().get_resource_system();
            resource_system.register_edit_callback(
                [this, &resource_system](Vadon::Scene::ResourceID resource_id)
                {
                    const VadonEditor::Model::ResourceInfo resource_info = resource_system.get_database().find_resource_info(resource_id);
                    if (Vadon::Utilities::TypeRegistry::is_base_of(Vadon::Utilities::TypeRegistry::get_type_id<VadonDemo::View::ViewResource>(), resource_info.info.type_id))
                    {
                        m_view->update_view_resource(VadonDemo::View::ViewResourceHandle::from_resource_handle(resource_system.get_resource(resource_id)->get_handle()));
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
        std::unique_ptr<VadonDemo::View::View> m_view;
        Vadon::Render::Canvas::RenderContext m_canvas_context;
    };
}

int main(int argc, char* argv[])
{
    Vadon::Core::EngineEnvironment engine_environment;

    Editor editor(engine_environment);
    return editor.execute(argc, argv);
}