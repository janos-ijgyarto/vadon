#include <VadonDemo/Model/Model.hpp>

#include <VadonDemo/Model/Component.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>
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

            if (init_renderer() == false)
            {
                return false;
            }

            if (init_ecs() == false)
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

                    m_canvas_context.camera.view_rectangle.position += get_delta_time() * 200 * camera_velocity;
                    m_canvas_context.camera.zoom = std::clamp(m_canvas_context.camera.zoom + get_delta_time() * camera_zoom * 10.0f, 0.1f, 10.0f);

                    // No multithreading here, so we render without any async
                    m_model->render_sync(ecs_world);

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

            component_manager.register_event_callback<VadonDemo::Model::Transform2D>(
                [this](const Vadon::ECS::ComponentEvent& event)
                {
                    component_event(event);
                }
            );
            component_manager.register_event_callback<VadonDemo::Model::CanvasComponent>(
                [this](const Vadon::ECS::ComponentEvent& event)
                {
                    component_event(event);
                }
            );

            editor_model.get_scene_system().register_edit_callback([this, &ecs_world](Vadon::ECS::EntityHandle entity, Vadon::Utilities::TypeID component)
                {
                    component_edited(entity, component);
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

        void component_event(const Vadon::ECS::ComponentEvent& event)
        {
            VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
            Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

            switch (event.event_type)
            {
            case Vadon::ECS::ComponentEventType::REMOVED:
            {
                if (event.type_id == Vadon::ECS::ComponentManager::get_component_type_id<VadonDemo::Model::CanvasComponent>())
                {
                    m_model->remove_canvas_item(ecs_world, event.owner);
                }
                break;
            }
            }
        }

        void component_edited(Vadon::ECS::EntityHandle entity, Vadon::ECS::ComponentID component)
        {
            VadonEditor::Model::ModelSystem& editor_model = get_system<VadonEditor::Model::ModelSystem>();
            Vadon::ECS::World& ecs_world = editor_model.get_ecs_world();

            if (component == Vadon::ECS::ComponentManager::get_component_type_id<VadonDemo::Model::CanvasComponent>())
            {
                m_model->update_canvas_item(ecs_world, entity);
            }
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
        Vadon::Render::Canvas::RenderContext m_canvas_context;
        Vadon::Utilities::PacketQueue m_view_queue;
    };
}

int main(int argc, char* argv[])
{
    Vadon::Core::EngineEnvironment engine_environment;

    Editor editor(engine_environment);
    return editor.execute(argc, argv);
}