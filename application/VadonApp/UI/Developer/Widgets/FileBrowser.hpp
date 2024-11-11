#ifndef VADONAPP_UI_DEVELOPER_WIDGETS_FILEBROWSER_HPP
#define VADONAPP_UI_DEVELOPER_WIDGETS_FILEBROWSER_HPP
#include <VadonApp/UI/Developer/Widgets/Dialog.hpp>
namespace VadonApp::UI::Developer
{
	// TODO: implement as generic widget to use in other contexts?
	class FileBrowserDialog : public Dialog
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
		};

		// TODO: flags?
		VADONAPP_API FileBrowserDialog(std::string_view title);

		void set_current_path(std::string_view path);

		VADONAPP_API File get_selected_file() const;
	protected:
		void on_open() override;
		Result internal_draw(GUISystem& dev_gui) override;
	private:
		void navigate_to_path(std::string_view path);

		bool open_selection();
		void navigate_up();

		std::string m_current_path;
		std::vector<std::string> m_current_dir_files;
		
		ListBox m_file_list_box;

		Button m_up_button;

		// TODO: make buttons configurable?
		Button m_open_button;
		Button m_cancel_button;
	};
}
#endif