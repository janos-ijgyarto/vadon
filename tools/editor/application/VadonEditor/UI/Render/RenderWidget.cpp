#include <VadonEditor/UI/Render/RenderWidget.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>

#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Platform.hpp>

#include <QResizeEvent>

namespace
{
	::Vadon::Foundation::PlatformKeyCode convert_qt_key_code(Qt::Key qt_key_code)
	{
		using PlatformKeyCode = ::Vadon::Foundation::PlatformKeyCode;
		switch (qt_key_code)
		{
		case Qt::Key_Return:
			return PlatformKeyCode::RETURN;
		case Qt::Key_Escape:
			return PlatformKeyCode::ESCAPE;
		case Qt::Key_Backspace:
			return PlatformKeyCode::BACKSPACE;
		case Qt::Key_Space:
			return PlatformKeyCode::SPACE;
		case Qt::Key_0:
			return PlatformKeyCode::KEY_0;
		case Qt::Key_1:
			return PlatformKeyCode::KEY_1;
		case Qt::Key_2:
			return PlatformKeyCode::KEY_2;
		case Qt::Key_3:
			return PlatformKeyCode::KEY_3;
		case Qt::Key_4:
			return PlatformKeyCode::KEY_4;
		case Qt::Key_5:
			return PlatformKeyCode::KEY_5;
		case Qt::Key_6:
			return PlatformKeyCode::KEY_6;
		case Qt::Key_7:
			return PlatformKeyCode::KEY_7;
		case Qt::Key_8:
			return PlatformKeyCode::KEY_8;
		case Qt::Key_9:
			return PlatformKeyCode::KEY_9;
		case Qt::Key_Dead_Grave:
			return PlatformKeyCode::BACKQUOTE;
		case Qt::Key_A:
			return PlatformKeyCode::KEY_a;
		case Qt::Key_B:
			return PlatformKeyCode::KEY_b;
		case Qt::Key_C:
			return PlatformKeyCode::KEY_c;
		case Qt::Key_D:
			return PlatformKeyCode::KEY_d;
		case Qt::Key_E:
			return PlatformKeyCode::KEY_e;
		case Qt::Key_F:
			return PlatformKeyCode::KEY_f;
		case Qt::Key_G:
			return PlatformKeyCode::KEY_g;
		case Qt::Key_H:
			return PlatformKeyCode::KEY_h;
		case Qt::Key_I:
			return PlatformKeyCode::KEY_i;
		case Qt::Key_J:
			return PlatformKeyCode::KEY_j;
		case Qt::Key_K:
			return PlatformKeyCode::KEY_k;
		case Qt::Key_L:
			return PlatformKeyCode::KEY_l;
		case Qt::Key_M:
			return PlatformKeyCode::KEY_m;
		case Qt::Key_N:
			return PlatformKeyCode::KEY_n;
		case Qt::Key_O:
			return PlatformKeyCode::KEY_o;
		case Qt::Key_P:
			return PlatformKeyCode::KEY_p;
		case Qt::Key_Q:
			return PlatformKeyCode::KEY_q;
		case Qt::Key_R:
			return PlatformKeyCode::KEY_r;
		case Qt::Key_S:
			return PlatformKeyCode::KEY_s;
		case Qt::Key_T:
			return PlatformKeyCode::KEY_t;
		case Qt::Key_U:
			return PlatformKeyCode::KEY_u;
		case Qt::Key_V:
			return PlatformKeyCode::KEY_v;
		case Qt::Key_W:
			return PlatformKeyCode::KEY_w;
		case Qt::Key_X:
			return PlatformKeyCode::KEY_x;
		case Qt::Key_Y:
			return PlatformKeyCode::KEY_y;
		case Qt::Key_Z:
			return PlatformKeyCode::KEY_z;
		case Qt::Key_Insert:
			return PlatformKeyCode::INSERT;
		case Qt::Key_Home:
			return PlatformKeyCode::HOME;
		case Qt::Key_PageUp:
			return PlatformKeyCode::PAGE_UP;
		case Qt::Key_Delete:
			return PlatformKeyCode::DELETE_KEY; // NOTE: needed suffix to avoid conflict with macro
		case Qt::Key_End:
			return PlatformKeyCode::END;
		case Qt::Key_PageDown:
			return PlatformKeyCode::PAGE_DOWN;
		case Qt::Key_Right:
			return PlatformKeyCode::RIGHT;
		case Qt::Key_Left:
			return PlatformKeyCode::LEFT;
		case Qt::Key_Down:
			return PlatformKeyCode::DOWN;
		case Qt::Key_Up:
			return PlatformKeyCode::UP;
		case Qt::Key_Control:
			return PlatformKeyCode::LEFT_CTRL;
		case Qt::Key_Shift:
			return PlatformKeyCode::LEFT_SHIFT;
		}

		return ::Vadon::Foundation::PlatformKeyCode::UNKNOWN;
	}

	::Vadon::Foundation::PlatformKeyModifiers convert_qt_key_modifiers(Qt::KeyboardModifiers qt_modifiers)
	{
		int modifiers = static_cast<int>(::Vadon::Foundation::PlatformKeyModifiers::NONE);
		if (qt_modifiers & Qt::ShiftModifier)
		{
			modifiers |= static_cast<int>(::Vadon::Foundation::PlatformKeyModifiers::SHIFT);
		}

		if (qt_modifiers & Qt::ControlModifier)
		{
			modifiers |= static_cast<int>(::Vadon::Foundation::PlatformKeyModifiers::CTRL);
		}

		if (qt_modifiers & Qt::AltModifier)
		{
			modifiers |= static_cast<int>(::Vadon::Foundation::PlatformKeyModifiers::ALT);
		}

		// TODO: other modifiers?
		return static_cast<::Vadon::Foundation::PlatformKeyModifiers>(modifiers);
	}

	::Vadon::Foundation::PlatformMouseButton convert_qt_mouse_button(Qt::MouseButton qt_mouse_button)
	{
		switch (qt_mouse_button)
		{
		case Qt::LeftButton:
			return ::Vadon::Foundation::PlatformMouseButton::LEFT;
		case Qt::RightButton:
			return ::Vadon::Foundation::PlatformMouseButton::RIGHT;
		case Qt::MiddleButton:
			return ::Vadon::Foundation::PlatformMouseButton::MIDDLE;
			// TODO: other buttons!
		}

		return ::Vadon::Foundation::PlatformMouseButton::INVALID;
	}
}

namespace VadonEditor::UI
{
	RenderWidget::RenderWidget(QWidget* parent)
		: QWidget(parent)
	{
		setFocusPolicy(Qt::FocusPolicy::ClickFocus);
	}

	RenderWidget::~RenderWidget()
	{

	}

	void RenderWidget::showEvent(QShowEvent* showEvent)
	{
		// TODO: anything?
		QWidget::showEvent(showEvent);
	}

	void RenderWidget::resizeEvent(QResizeEvent* resizeEvent)
	{
		if (m_client_info.is_valid() == true)
		{
			::Vadon::Foundation::EditorPlatformEventHeader platform_event_header;
			platform_event_header.message_type = ::Vadon::Foundation::EditorPlatformMessageType::PLATFORM_EVENT;
			platform_event_header.event_type = ::Vadon::Foundation::PlatformEventType::WINDOW;

			::Vadon::Foundation::PlatformWindowEvent window_event;
			window_event.type = ::Vadon::Foundation::PlatformWindowEventType::RESIZED;
			window_event.window_id = m_client_info.client_id;

			window_event.data1 = resizeEvent->size().width();
			window_event.data2 = resizeEvent->size().height();

			VadonEditor::Network::MessageSerializer message_serializer;
			char* data_ptr = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::PLATFORM, sizeof(::Vadon::Foundation::EditorPlatformEventHeader) + sizeof(::Vadon::Foundation::PlatformWindowEvent));

			memcpy(data_ptr, &platform_event_header, sizeof(::Vadon::Foundation::EditorPlatformEventHeader));
			memcpy(data_ptr + sizeof(::Vadon::Foundation::EditorPlatformEventHeader), &window_event, sizeof(::Vadon::Foundation::PlatformWindowEvent));

			m_client_info.application->get_network_system().send_message(message_serializer);
		}

		QWidget::resizeEvent(resizeEvent);
	}

	void RenderWidget::paintEvent(QPaintEvent* paintEvent)
	{
		// TODO: anything?
		QWidget::paintEvent(paintEvent);
	}

	void RenderWidget::keyPressEvent(QKeyEvent* event)
	{
		if (m_client_info.is_valid() == true)
		{
			::Vadon::Foundation::EditorPlatformEventHeader platform_event_header;
			platform_event_header.message_type = ::Vadon::Foundation::EditorPlatformMessageType::PLATFORM_EVENT;
			platform_event_header.event_type = ::Vadon::Foundation::PlatformEventType::KEYBOARD;

			::Vadon::Foundation::PlatformKeyboardEvent keyboard_event;
			keyboard_event.key = convert_qt_key_code(static_cast<Qt::Key>(event->key()));

			keyboard_event.down = true;
			keyboard_event.modifiers = convert_qt_key_modifiers(event->modifiers());
			keyboard_event.native_code = event->nativeVirtualKey();
			keyboard_event.native_scancode = event->nativeScanCode();

			VadonEditor::Network::MessageSerializer message_serializer;
			char* data_ptr = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::PLATFORM, sizeof(::Vadon::Foundation::EditorPlatformEventHeader) + sizeof(::Vadon::Foundation::PlatformKeyboardEvent));

			memcpy(data_ptr, &platform_event_header, sizeof(::Vadon::Foundation::EditorPlatformEventHeader));
			memcpy(data_ptr + sizeof(::Vadon::Foundation::EditorPlatformEventHeader), &keyboard_event, sizeof(::Vadon::Foundation::PlatformKeyboardEvent));

			m_client_info.application->get_network_system().send_message(message_serializer);
		}

		QWidget::keyPressEvent(event);
	}

	void RenderWidget::keyReleaseEvent(QKeyEvent* event)
	{
		if (event->isAutoRepeat() == true)
		{
			event->ignore();
			return;
		}

		if (m_client_info.is_valid() == true)
		{
			::Vadon::Foundation::EditorPlatformEventHeader platform_event_header;
			platform_event_header.message_type = ::Vadon::Foundation::EditorPlatformMessageType::PLATFORM_EVENT;
			platform_event_header.event_type = ::Vadon::Foundation::PlatformEventType::KEYBOARD;

			::Vadon::Foundation::PlatformKeyboardEvent keyboard_event;
			keyboard_event.key = convert_qt_key_code(static_cast<Qt::Key>(event->key()));

			keyboard_event.down = false;
			keyboard_event.modifiers = convert_qt_key_modifiers(event->modifiers());
			keyboard_event.native_code = event->nativeVirtualKey();
			keyboard_event.native_scancode = event->nativeScanCode();

			VadonEditor::Network::MessageSerializer message_serializer;
			char* data_ptr = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::PLATFORM, sizeof(::Vadon::Foundation::EditorPlatformEventHeader) + sizeof(::Vadon::Foundation::PlatformKeyboardEvent));

			memcpy(data_ptr, &platform_event_header, sizeof(::Vadon::Foundation::EditorPlatformEventHeader));
			memcpy(data_ptr + sizeof(::Vadon::Foundation::EditorPlatformEventHeader), &keyboard_event, sizeof(::Vadon::Foundation::PlatformKeyboardEvent));

			m_client_info.application->get_network_system().send_message(message_serializer);
		}

		QWidget::keyReleaseEvent(event);
	}

	void RenderWidget::mousePressEvent(QMouseEvent* event)
	{
		if (m_client_info.is_valid() == true)
		{
			::Vadon::Foundation::EditorPlatformEventHeader platform_event_header;
			platform_event_header.message_type = ::Vadon::Foundation::EditorPlatformMessageType::PLATFORM_EVENT;
			platform_event_header.event_type = ::Vadon::Foundation::PlatformEventType::MOUSE_BUTTON;

			::Vadon::Foundation::PlatformMouseButtonEvent mouse_button_event;
			mouse_button_event.button = convert_qt_mouse_button(event->button());
			mouse_button_event.down = true;

			VadonEditor::Network::MessageSerializer message_serializer;
			char* data_ptr = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::PLATFORM, sizeof(::Vadon::Foundation::EditorPlatformEventHeader) + sizeof(::Vadon::Foundation::PlatformMouseButtonEvent));

			memcpy(data_ptr, &platform_event_header, sizeof(::Vadon::Foundation::EditorPlatformEventHeader));
			memcpy(data_ptr + sizeof(::Vadon::Foundation::EditorPlatformEventHeader), &mouse_button_event, sizeof(::Vadon::Foundation::PlatformMouseButtonEvent));

			m_client_info.application->get_network_system().send_message(message_serializer);
		}

		QWidget::mousePressEvent(event);
	}

	void RenderWidget::mouseReleaseEvent(QMouseEvent* event)
	{
		if (m_client_info.is_valid() == true)
		{
			::Vadon::Foundation::EditorPlatformEventHeader platform_event_header;
			platform_event_header.message_type = ::Vadon::Foundation::EditorPlatformMessageType::PLATFORM_EVENT;
			platform_event_header.event_type = ::Vadon::Foundation::PlatformEventType::MOUSE_BUTTON;

			::Vadon::Foundation::PlatformMouseButtonEvent mouse_button_event;
			mouse_button_event.button = convert_qt_mouse_button(event->button());
			mouse_button_event.down = false;

			VadonEditor::Network::MessageSerializer message_serializer;
			char* data_ptr = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::PLATFORM, sizeof(::Vadon::Foundation::EditorPlatformEventHeader) + sizeof(::Vadon::Foundation::PlatformMouseButtonEvent));

			memcpy(data_ptr, &platform_event_header, sizeof(::Vadon::Foundation::EditorPlatformEventHeader));
			memcpy(data_ptr + sizeof(::Vadon::Foundation::EditorPlatformEventHeader), &mouse_button_event, sizeof(::Vadon::Foundation::PlatformMouseButtonEvent));

			m_client_info.application->get_network_system().send_message(message_serializer);
		}

		QWidget::mouseReleaseEvent(event);
	}

	void RenderWidget::mouseMoveEvent(QMouseEvent* event)
	{
		if (m_client_info.is_valid() == true)
		{
			::Vadon::Foundation::EditorPlatformEventHeader platform_event_header;
			platform_event_header.message_type = ::Vadon::Foundation::EditorPlatformMessageType::PLATFORM_EVENT;
			platform_event_header.event_type = ::Vadon::Foundation::PlatformEventType::MOUSE_MOTION;

			::Vadon::Foundation::PlatformMouseMotionEvent mouse_motion_event;
			mouse_motion_event.position_x = event->pos().x();
			mouse_motion_event.position_y = event->pos().y();
			// TODO: add relative motion!

			VadonEditor::Network::MessageSerializer message_serializer;
			char* data_ptr = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::PLATFORM, sizeof(::Vadon::Foundation::EditorPlatformEventHeader) + sizeof(::Vadon::Foundation::PlatformMouseMotionEvent));

			memcpy(data_ptr, &platform_event_header, sizeof(::Vadon::Foundation::EditorPlatformEventHeader));
			memcpy(data_ptr + sizeof(::Vadon::Foundation::EditorPlatformEventHeader), &mouse_motion_event, sizeof(::Vadon::Foundation::PlatformMouseMotionEvent));

			m_client_info.application->get_network_system().send_message(message_serializer);
		}

		QWidget::mouseMoveEvent(event);
	}

	void RenderWidget::wheelEvent(QWheelEvent* event)
	{
		if (m_client_info.is_valid() == true)
		{
			::Vadon::Foundation::EditorPlatformEventHeader platform_event_header;
			platform_event_header.message_type = ::Vadon::Foundation::EditorPlatformMessageType::PLATFORM_EVENT;
			platform_event_header.event_type = ::Vadon::Foundation::PlatformEventType::MOUSE_WHEEL;

			::Vadon::Foundation::PlatformMouseWheelEvent mouse_wheel_event;

			// FIXME: logic below taken from Qt documentation
			// Update if needed!
			const QPoint numPixels = event->pixelDelta();

			constexpr int c_delta_step_size = 8;
			const QPoint numDegrees = event->angleDelta() / c_delta_step_size;

			if (numPixels.isNull() == false)
			{
				mouse_wheel_event.x = numPixels.x();
				mouse_wheel_event.y = numPixels.y();
			}
			else if (numDegrees.isNull() == false)
			{
				constexpr int c_step_degrees = 15;
				QPoint numSteps = numDegrees / c_step_degrees;

				mouse_wheel_event.x = numSteps.x();
				mouse_wheel_event.y = numSteps.y();
			}

			// TODO: get precise values!
			mouse_wheel_event.precise_x = mouse_wheel_event.x;
			mouse_wheel_event.precise_y = mouse_wheel_event.y;

			VadonEditor::Network::MessageSerializer message_serializer;
			char* data_ptr = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::PLATFORM, sizeof(::Vadon::Foundation::EditorPlatformEventHeader) + sizeof(::Vadon::Foundation::PlatformMouseWheelEvent));

			memcpy(data_ptr, &platform_event_header, sizeof(::Vadon::Foundation::EditorPlatformEventHeader));
			memcpy(data_ptr + sizeof(::Vadon::Foundation::EditorPlatformEventHeader), &mouse_wheel_event, sizeof(::Vadon::Foundation::PlatformMouseWheelEvent));

			m_client_info.application->get_network_system().send_message(message_serializer);
		}

		QWidget::wheelEvent(event);
	}
}