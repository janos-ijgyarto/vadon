#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/UI/Developer/Widgets/FileBrowser.hpp>

#include <VadonApp/UI/Developer/GUI.hpp>
#include <VadonApp/UI/Developer/IconsFontAwesome5.h>

#include <filesystem>

namespace VadonApp::UI::Developer
{
    FileBrowserDialog::FileBrowserDialog(std::string_view title)
        : Dialog(title)
    {
        m_file_list_box.label = "Browser";
        m_file_list_box.size = { -1, -1 };

        m_up_button.label = ICON_FA_ARROW_UP;

        m_open_button.label = "Open";
        m_cancel_button.label = "Cancel";
    }

    void FileBrowserDialog::set_current_path(std::string_view path)
    {
        m_current_path = path;
    }

    FileBrowserDialog::File FileBrowserDialog::get_selected_file() const
    {
        if (m_file_list_box.has_valid_selection() == true)
        {
            std::filesystem::path path = m_current_path;
            path /= m_current_dir_files[m_file_list_box.selected_item];

            File selected_file;
            selected_file.path = path.string();

            if (std::filesystem::is_regular_file(path) == true)
            {
                selected_file.type = Type::FILE;
            }
            else if (std::filesystem::is_directory(path) == true)
            {
                selected_file.type = Type::DIRECTORY;
            }

            return selected_file;
        }

        return File();
    }

    void FileBrowserDialog::on_open()
    {
        if (m_current_path.empty() == true)
        {
            navigate_to_path(std::filesystem::current_path().string());
        }

        navigate_to_path(m_current_path);
    }

	Dialog::Result FileBrowserDialog::internal_draw(GUISystem& dev_gui)
	{
        Result result = Result::NONE;

        dev_gui.add_text(m_current_path);

        if (dev_gui.draw_button(m_up_button) == true)
        {
            navigate_up();
        }

        Vadon::Utilities::Vector2 file_list_area = dev_gui.get_available_content_region();
        const GUIStyle gui_style = dev_gui.get_style();
        file_list_area.y -= dev_gui.calculate_text_size(m_open_button.label).y + gui_style.frame_padding.y * 2 + 5.0f;

        m_file_list_box.size = file_list_area;

        bool file_double_clicked = false;
        dev_gui.draw_list_box(m_file_list_box, &file_double_clicked);

        const bool file_selected = m_file_list_box.has_valid_selection();
        if (file_selected && file_double_clicked)
        {
            open_selection();
        }
        if (file_selected == false)
        {
            dev_gui.begin_disabled(true);
        }
        if (dev_gui.draw_button(m_open_button) == true)
        {
            result = Result::ACCEPTED;
        }
        if (file_selected == false)
        {
            dev_gui.end_disabled();
        }
        dev_gui.same_line();
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

    void FileBrowserDialog::navigate_to_path(std::string_view path)
    {
        if (path.empty() == true)
        {
            // TODO: error?
            return;
        }

        if (path == m_current_path)
        {
            return;
        }

        if (std::filesystem::is_directory(path) == false)
        {
            // TODO: error!
            return;
        }

        m_current_path = path;
        m_current_dir_files.clear();

        m_file_list_box.deselect();
        m_file_list_box.items.clear();

        std::vector<std::filesystem::path> sorted_path_list;
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_directory() == true)
            {
                sorted_path_list.push_back(entry);
            }
            else if (entry.is_regular_file() == true)
            {
                sorted_path_list.push_back(entry);
            }
        }

        std::sort(sorted_path_list.begin(), sorted_path_list.end(),
            [](const std::filesystem::path& lhs, const std::filesystem::path& rhs)
            {
                const bool lhs_dir = std::filesystem::is_directory(lhs);
                const bool rhs_dir = std::filesystem::is_directory(rhs);
                if (lhs_dir)
                {
                    if (rhs_dir)
                    {
                        return lhs < rhs;
                    }
                    else
                    {
                        return true;
                    }
                }
                else if (rhs_dir)
                {
                    return false;
                }
                return lhs < rhs;
            }
        );

        for (const auto& current_path : sorted_path_list)
        {
            if (std::filesystem::is_directory(current_path) == true)
            {
                m_current_dir_files.push_back(current_path.filename().string());
                m_file_list_box.items.push_back(ICON_FA_FOLDER + (" " + m_current_dir_files.back()));
            }
            else if (std::filesystem::is_regular_file(current_path) == true)
            {
                m_current_dir_files.push_back(current_path.filename().string());
                m_file_list_box.items.push_back(ICON_FA_FILE + (" " + m_current_dir_files.back()));
            }
        }
    }

    bool FileBrowserDialog::open_selection()
    {
        std::filesystem::path current_path = m_current_path;
        current_path /= m_current_dir_files[m_file_list_box.selected_item];

        if (std::filesystem::is_directory(current_path) == true)
        {
            navigate_to_path(current_path.string());
        }

        return false;
    }

    void FileBrowserDialog::navigate_up()
    {
        std::filesystem::path current_path = m_current_path;
        if (current_path.has_parent_path())
        {
            navigate_to_path(current_path.parent_path().string());
        }
    }
}