#ifndef VADONAPP_UI_DEVELOPER_WIDGETS_FILEBROWSER_HPP
#define VADONAPP_UI_DEVELOPER_WIDGETS_FILEBROWSER_HPP
#include <VadonApp/VadonApp.hpp>
#include <VadonApp/UI/Developer/Widgets/Dialog.hpp>
namespace VadonApp::UI::Developer
{
	class GUISystem;

	// TODO: implement model/view separation where the browser is a "model" (with no GUI logic)
	// and the GUI here is just a "view" on it?
	class FileBrowserWidget
	{
	public:
		enum class Type
		{
			NONE,
			FILE,
			DIRECTORY
		};

		struct File
		{
			Type type = Type::NONE;
			std::string path;
			bool double_clicked = false;
		};

		VADONAPP_API FileBrowserWidget(std::string_view label = "#file_browser");

		// TODO: add search bar!
		VADONAPP_API void navigate_to_path(std::string_view path);

		const std::string& get_current_path() const { return m_current_path; }
		const std::vector<std::string>& get_current_dir_files() const { return m_current_dir_files; }

		const File& get_selected_file() const { return m_selected_file; }
		void reset_selection();

		// TODO: some way to extract a "clicked" event besides double click
		VADONAPP_API void draw(GUISystem& dev_gui, const Vadon::Math::Vector2& size = { -1, -1 });
		void refresh_current_directory_view();
	private:
		void navigate_up();

		void update_selection();
		void internal_navigate_to_path(std::string_view path);

		// TODO: filter for file extensions

		std::string m_current_path;
		std::vector<std::string> m_current_dir_files;
		File m_selected_file;

		ListBox m_file_list_box;
		Button m_up_button;
	};

	class FileBrowserDialog : public Dialog
	{
	public:
		enum class Flags
		{
			NAME_INPUT = 1 << 0,
			DEFAULT = 0
		};
		VADONAPP_API FileBrowserDialog(std::string_view title = "File Browser", Flags flags = Flags::DEFAULT);

		void set_title(std::string_view title) { m_window.title = title; }

		const FileBrowserWidget::File& get_selected_file() const { return m_file_browser.get_selected_file(); }
		const std::string& get_current_path() const { return m_file_browser.get_current_path(); }
		
		const std::string& get_file_name_input() const { return m_file_name_input.input; }
		void set_file_name_input(std::string_view name) { m_file_name_input.input = name; }
		void clear_file_name_input() { m_file_name_input.input.clear(); }

		// TODO: add file extension!
		VADONAPP_API std::string get_entered_file_path() const;

		void set_accept_label(std::string_view label) { m_accept_button.label = label; }

		VADONAPP_API void open_at(std::string_view path);
	protected:
		Result internal_draw(GUISystem& dev_gui) override;

		Flags m_flags;

		FileBrowserWidget m_file_browser;

		// TODO: combo for file extensions
		InputText m_file_name_input;

		Button m_accept_button;
		Button m_cancel_button;
	};
}

namespace Vadon::Utilities
{
	template<>
	struct EnableEnumBitwiseOperators<VadonApp::UI::Developer::FileBrowserDialog::Flags> : public std::true_type
	{

	};
}
#endif