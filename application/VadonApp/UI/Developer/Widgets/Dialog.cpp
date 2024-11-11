#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/UI/Developer/Widgets/Dialog.hpp>

#include <VadonApp/UI/Developer/GUI.hpp>

namespace VadonApp::UI::Developer
{
	Dialog::Dialog(std::string_view title)
		: m_state(State::CLOSED)
	{
		m_window.title = title;
	}

	void Dialog::open()
	{
		if (m_state == State::CLOSED)
		{
			m_state = State::OPEN_REQUESTED;
		}
	}

	void Dialog::close()
	{
		if (m_state == State::OPEN)
		{
			m_state = State::CLOSE_REQUESTED;
		}
	}

	Dialog::Result Dialog::draw(GUISystem& dev_gui)
	{
		Result result = Result::INACTIVE;

		switch (m_state)
		{
		case State::CLOSED:
			return result;
		case State::OPEN_REQUESTED:
		{
			dev_gui.open_dialog(m_window.title);
			m_state = State::OPEN;
			on_open();
			break;
		}
		}

		if (dev_gui.begin_modal_dialog(m_window) == true)
		{
			result = internal_draw(dev_gui);
			if (m_state == State::CLOSE_REQUESTED)
			{
				m_state = State::CLOSED;
				dev_gui.close_current_dialog();
				// TODO: on_close?
			}

			dev_gui.end_dialog();
		}
		else
		{
			if (m_state != State::CLOSED)
			{
				// TODO!
			}
		}

		return result;
	}

	MessageDialog::MessageDialog(std::string_view title, std::string_view message)
		: Dialog(title)
		, m_message(message)
	{
		m_ok_button.label = "Ok";
	}

	Dialog::Result MessageDialog::internal_draw(VadonApp::UI::Developer::GUISystem& dev_gui)
	{
		dev_gui.add_text(m_message);
		if (dev_gui.draw_button(m_ok_button) == true)
		{
			close();
			return Result::ACCEPTED;
		}

		return Result::NONE;
	}

	ConfirmDialog::ConfirmDialog(std::string_view title, std::string_view message)
		: Dialog(title)
		, m_message(message)
	{
		m_yes_button.label = "Yes";
		m_no_button.label = "No";
		m_cancel_button.label = "Cancel";
	}

	Dialog::Result ConfirmDialog::internal_draw(VadonApp::UI::Developer::GUISystem& dev_gui)
	{
		Result result = Result::NONE;

		dev_gui.add_text(m_message);

		if (dev_gui.draw_button(m_yes_button) == true)
		{
			result = Result::ACCEPTED;
		}
		if (dev_gui.draw_button(m_no_button) == true)
		{
			result = Result::REJECTED;
		}
		if (dev_gui.draw_button(m_cancel_button) == true)
		{
			result = Result::CANCELLED;
		}
		if (result != Result::NONE)
		{
			close();
		}

		return result;
	}
}