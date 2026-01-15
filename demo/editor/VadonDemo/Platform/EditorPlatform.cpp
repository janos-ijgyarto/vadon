#include <VadonDemo/Platform/EditorPlatform.hpp>

#include <VadonDemo/Core/Editor.hpp>

#include <VadonDemo/Network/Message/MessageSerializer.hpp>

#include <Vadon/Foundation/Editor/SimulatorInterface.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Platform.hpp>

namespace VadonDemo::Platform
{
    EditorPlatform::EditorPlatform(Core::Editor& editor)
        : m_editor(editor)
        , m_actions_pressed{}
        , m_action_strengths{}
    {

    }

    bool EditorPlatform::initialize()
    {
        // TODO: anything else?

        // TODO2: integrate input system from Application!
        return true;
    }

    void EditorPlatform::update()
    {
        // Reset zoom (so it doesn't get "stuck")
        set_action_strength(EditorInputAction::CAMERA_ZOOM, 0.0f);
    }

    void EditorPlatform::editor_connected()
    {
        request_main_viewport();
    }

    void EditorPlatform::process_message(const char* data, size_t size)
    {
        ::Vadon::Foundation::EditorMessageReader message_reader(data, size);
        switch (message_reader.get_current_category())
        {
        case ::Vadon::Foundation::EditorMessageCategory::PLATFORM:
        {
            const char* message_data = message_reader.get_current_message_data();
            const ::Vadon::Foundation::EditorPlatformMessageHeader* platform_message_header = reinterpret_cast<const ::Vadon::Foundation::EditorPlatformMessageHeader*>(message_data);
            switch (platform_message_header->message_type)
            {
            case ::Vadon::Foundation::EditorPlatformMessageType::MANAGER_WINDOW_REQUEST:
            {
                const ::Vadon::Foundation::EditorPlatformManagerWindowRequest* window_request = reinterpret_cast<const ::Vadon::Foundation::EditorPlatformManagerWindowRequest*>(message_data);
                m_window.platform_handle = window_request->handle;
            }
            break;
            case ::Vadon::Foundation::EditorPlatformMessageType::PLATFORM_EVENT:
            {
                const ::Vadon::Foundation::EditorPlatformEventHeader* platform_event_header = reinterpret_cast<const ::Vadon::Foundation::EditorPlatformEventHeader*>(message_data);
                const char* platform_event_data = message_data + sizeof(::Vadon::Foundation::EditorPlatformEventHeader);
                switch (platform_event_header->event_type)
                {
                case ::Vadon::Foundation::PlatformEventType::WINDOW:
                {
                    // TODO: check whether window ID matches!
                    const ::Vadon::Foundation::PlatformWindowEvent* window_event = reinterpret_cast<const ::Vadon::Foundation::PlatformWindowEvent*>(platform_event_data);
                    using WindowEventType = ::Vadon::Foundation::PlatformWindowEventType;
                    switch (window_event->type)
                    {
                    case WindowEventType::RESIZED:
                    {
                        m_window.size.x = window_event->data1;
                        m_window.size.y = window_event->data2;
                    }
                    break;
                    }
                }
                break;
                case ::Vadon::Foundation::PlatformEventType::KEYBOARD:
                {
                    using KeyCode = ::Vadon::Foundation::PlatformKeyCode;
                    const ::Vadon::Foundation::PlatformKeyboardEvent* key_event = reinterpret_cast<const ::Vadon::Foundation::PlatformKeyboardEvent*>(platform_event_data);
                    switch (key_event->key)
                    {
                    case KeyCode::UP:
                        set_action_pressed(EditorInputAction::CAMERA_UP, key_event->down);
                        set_action_strength(EditorInputAction::CAMERA_UP, 1.0f);
                        break;
                    case KeyCode::DOWN:
                        set_action_pressed(EditorInputAction::CAMERA_DOWN, key_event->down);
                        set_action_strength(EditorInputAction::CAMERA_DOWN, 1.0f);
                        break;
                    case KeyCode::LEFT:
                        set_action_pressed(EditorInputAction::CAMERA_LEFT, key_event->down);
                        set_action_strength(EditorInputAction::CAMERA_LEFT, 1.0f);
                        break;
                    case KeyCode::RIGHT:
                        set_action_pressed(EditorInputAction::CAMERA_RIGHT, key_event->down);
                        set_action_strength(EditorInputAction::CAMERA_RIGHT, 1.0f);
                        break;
                    }
                }
                break;
                case ::Vadon::Foundation::PlatformEventType::MOUSE_WHEEL:
                {
                    const ::Vadon::Foundation::PlatformMouseWheelEvent* wheel_event = reinterpret_cast<const ::Vadon::Foundation::PlatformMouseWheelEvent*>(platform_event_data);
                    set_action_strength(EditorInputAction::CAMERA_ZOOM, wheel_event->y > 0 ? 1.0f : -1.0f);
                }
                break;
                }
            }
            break;
            }
        }
        break;
        }
    }

    void EditorPlatform::request_main_viewport()
    {
        Network::MessageSerializer message_serializer;

        // FIXME: actually make use of ID!
        ::Vadon::Foundation::EditorPlatformManagerWindowRequest window_request;
        window_request.message_type = ::Vadon::Foundation::EditorPlatformMessageType::MANAGER_WINDOW_REQUEST;
        window_request.id = 1;

        message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::PLATFORM, window_request);

        m_editor.get_simulator().dispatch_message_to_editor(message_serializer.get_buffer().data(), message_serializer.get_buffer().size());
    }
}