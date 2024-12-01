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
		VADONAPP_API void draw(GUISystem& dev_gui, const Vadon::Utilities::Vector2& size = { -1, -1 });
		void refresh_current_directory_view();
	private:
		void navigate_up();

		void update_selection();
		void internal_navigate_to_path(std::string_view path);

		std::string m_current_path;
		std::vector<std::string> m_current_dir_files;
		File m_selected_file;

		ListBox m_file_list_box;
		Button m_up_button;
	};

	class FileBrowserDialog : public Dialog
	{
	public:
		// TODO: modes, flags, extensions, etc?
		VADONAPP_API FileBrowserDialog(std::string_view title);

		const FileBrowserWidget::File& get_selected_file() const { return m_file_browser.get_selected_file(); }
		const std::string& get_current_directory() const { return m_file_browser.get_current_path(); }
		VADONAPP_API void open_at(std::string_view path);
	protected:
		Result internal_draw(GUISystem& dev_gui) override;

		FileBrowserWidget m_file_browser;

		// TODO: extension selector?

		Button m_select_button;
		Button m_cancel_button;
	};
}
#endif