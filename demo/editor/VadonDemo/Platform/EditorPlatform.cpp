#include <VadonDemo/Platform/EditorPlatform.hpp>

#include <VadonDemo/Core/Editor.hpp>

#include <VadonApp/Platform/Input/InputSystem.hpp>

namespace VadonDemo::Platform
{
    EditorPlatform::EditorPlatform(Core::Editor& editor)
        : m_editor(editor)
    {

    }

    bool EditorPlatform::initialize()
    {
        VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
        VadonApp::Platform::InputSystem& input_system = engine_app.get_system<VadonApp::Platform::InputSystem>();

        {
            VadonApp::Platform::InputActionHandle camera_up_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_up" });
            input_system.add_key_entry(camera_up_action, VadonApp::Platform::KeyCode::UP);
            m_input_actions[Vadon::Utilities::to_integral(EditorInputAction::CAMERA_UP)] = camera_up_action;
        }

        {
            VadonApp::Platform::InputActionHandle camera_down_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_down" });
            input_system.add_key_entry(camera_down_action, VadonApp::Platform::KeyCode::DOWN);
            m_input_actions[Vadon::Utilities::to_integral(EditorInputAction::CAMERA_DOWN)] = camera_down_action;
        }

        {
            VadonApp::Platform::InputActionHandle camera_left_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_left" });
            input_system.add_key_entry(camera_left_action, VadonApp::Platform::KeyCode::LEFT);
            m_input_actions[Vadon::Utilities::to_integral(EditorInputAction::CAMERA_LEFT)] = camera_left_action;
        }

        {
            VadonApp::Platform::InputActionHandle camera_right_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_right" });
            input_system.add_key_entry(camera_right_action, VadonApp::Platform::KeyCode::RIGHT);
            m_input_actions[Vadon::Utilities::to_integral(EditorInputAction::CAMERA_RIGHT)] = camera_right_action;
        }

        {
            VadonApp::Platform::InputActionHandle camera_zoom_action = input_system.create_input_action(VadonApp::Platform::InputActionInfo{ .name = "camera_zoom" });
            input_system.add_mouse_wheel_entry(camera_zoom_action);
            m_input_actions[Vadon::Utilities::to_integral(EditorInputAction::CAMERA_ZOOM)] = camera_zoom_action;
        }

        return true;
    }
}