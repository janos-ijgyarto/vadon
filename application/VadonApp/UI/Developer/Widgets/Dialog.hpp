#ifndef VADONAPP_UI_DEVELOPER_WIDGETS_DIALOG_HPP
#define VADONAPP_UI_DEVELOPER_WIDGETS_DIALOG_HPP
#include <VadonApp/VadonApp.hpp>
#include <VadonApp/UI/Developer/GUIElements.hpp>
namespace VadonApp::UI::Developer
{
	class GUISystem;

	// TODO: move these to Application as reusable utility widgets!
	class Dialog
	{
	public:
		// FIXME: merge dialog API with common return types?
		enum class Result
		{
			INACTIVE,
			NONE,
			ACCEPTED,
			REJECTED,
			CANCELLED
		};

		VADONAPP_API Dialog(std::string_view title = "");
		virtual ~Dialog() {}

		const std::string& get_title() const { return m_window.title; }
		void set_title(std::string_view title) { m_window.title = title; }

		VADONAPP_API void open();
		VADONAPP_API void close();

		VADONAPP_API Result draw(GUISystem& dev_gui);
	protected:
		enum class State
		{
			CLOSED,
			OPEN_REQUESTED,
			OPEN,
			CLOSE_REQUESTED
		};

		virtual void on_open() {}
		virtual Result internal_draw(GUISystem&) { return Result::NONE; }

		Window m_window;
		State m_state;
	};

	class MessageDialog : public Dialog
	{
	public:
		VADONAPP_API MessageDialog(std::string_view title = "", std::string_view message = "");

		const std::string& get_message() const { return m_message; }
		void set_message(std::string_view title) { m_message = title; }
	protected:
		Result internal_draw(VadonApp::UI::Developer::GUISystem& dev_gui) override;
	private:
		std::string m_message;
		Button m_ok_button; // FIXME: this could be shared across all instances?
	};

	class ConfirmDialog : public Dialog
	{
	public:
		// TODO: flags?
		VADONAPP_API ConfirmDialog(std::string_view title, std::string_view message);
	protected:
		Result internal_draw(VadonApp::UI::Developer::GUISystem& dev_gui) override;
	private:
		std::string m_message;
		Button m_yes_button;
		Button m_no_button;
		Button m_cancel_button;
	};
}
#endif