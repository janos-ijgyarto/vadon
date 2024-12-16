#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/UI/Developer/Widgets/FileBrowser.hpp>

#include <VadonApp/UI/Developer/GUI.hpp>
#include <VadonApp/UI/Developer/IconsFontAwesome5.h>

#include <filesystem>

namespace VadonApp::UI::Developer
{
    FileBrowserWidget::FileBrowserWidget(std::string_view label)
    {
        m_file_list_box.label = label;
        m_file_list_box.size = { -1, -1 };

        m_up_button.label = ICON_FA_ARROW_UP;
    }

    void FileBrowserWidget::navigate_to_path(std::string_view path)
    {
        if (std::filesystem::is_directory(path) == false)
        {
            internal_navigate_to_path(std::filesystem::path(path).parent_path().string());
        }
        else
        {
            internal_navigate_to_path(path);
        }
    }

    void FileBrowserWidget::reset_selection()
    {
        m_selected_file.type = Type::NONE;
        m_selected_file.path.clear();
        m_selected_file.double_clicked = false;
    }

    void FileBrowserWidget::draw(GUISystem& dev_gui, const Vadon::Utilities::Vector2& size)
	{
        if (dev_gui.draw_button(m_up_button) == true)
        {
            navigate_up();
        }
        dev_gui.same_line();
        dev_gui.add_text(m_current_path);

        m_file_list_box.size = size;

        const GUIStyle gui_style = dev_gui.get_style();
        m_file_list_box.size.y -= dev_gui.calculate_text_size(m_up_button.label).y + gui_style.frame_padding.y * 2;
        m_file_list_box.size.y -= dev_gui.calculate_text_size(m_current_path).y + gui_style.frame_padding.y * 2;

        const int32_t prev_selection = m_file_list_box.selected_item;
        dev_gui.draw_list_box(m_file_list_box, &m_selected_file.double_clicked);

        if (prev_selection != m_file_list_box.selected_item)
        {
            update_selection();
        }

        if ((m_selected_file.type == Type::DIRECTORY) && (m_selected_file.double_clicked == true))
        {
            navigate_to_path(m_selected_file.path);
        }
	}

    void FileBrowserWidget::refresh_current_directory_view()
    {
        m_current_dir_files.clear();

        reset_selection();
        m_file_list_box.deselect();
        m_file_list_box.items.clear();

        std::vector<std::filesystem::path> sorted_path_list;
        for (const auto& entry : std::filesystem::directory_iterator(m_current_path))
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

    void FileBrowserWidget::internal_navigate_to_path(std::string_view path)
    {
        if (path.empty() == true)
        {
            Vadon::Core::Logger::log_error("File browser error: invalid path!\n");
            return;
        }

        if (path == m_current_path)
        {
            return;
        }

        m_current_path = path;

        refresh_current_directory_view();
    }

    void FileBrowserWidget::navigate_up()
    {
        std::filesystem::path current_path = m_current_path;
        if (current_path.has_parent_path())
        {
            navigate_to_path(current_path.parent_path().string());
        }
    }

    void FileBrowserWidget::update_selection()
    {
        if (m_file_list_box.has_valid_selection() == true)
        {
            std::filesystem::path path = m_current_path;
            path /= m_current_dir_files[m_file_list_box.selected_item];

            m_selected_file.path = path.string();
            if (std::filesystem::is_directory(path) == true)
            {
                m_selected_file.type = Type::DIRECTORY;
            }
            else
            {
                m_selected_file.type = Type::FILE;
            }
        }
        else
        {
            reset_selection();
        }
    }

    FileBrowserDialog::FileBrowserDialog(std::string_view title, Flags flags)
        : Dialog(title)
        , m_flags(flags)
    {
        m_accept_button.label = "Select";
        m_cancel_button.label = "Cancel";

        m_file_name_input.label = "File name";
    }

    std::string FileBrowserDialog::get_entered_file_path() const
    {
        return (std::filesystem::path(m_file_browser.get_current_path()) / m_file_name_input.input).string();
    }

    void FileBrowserDialog::open_at(std::string_view path)
    {
        if (path.empty() == true)
        {
            Vadon::Core::Logger::log_error("File browser error: invalid path!\n");
            return;
        }

        m_file_browser.navigate_to_path(path);
        open();
    }

    Dialog::Result FileBrowserDialog::internal_draw(GUISystem& dev_gui)
    {
        Result result = Result::NONE;

        Vadon::Utilities::Vector2 file_browser_area = dev_gui.get_available_content_region();
        const GUIStyle gui_style = dev_gui.get_style();
        file_browser_area.y -= dev_gui.calculate_text_size(m_accept_button.label).y + gui_style.frame_padding.y * 2 + 5.0f;
        if (Vadon::Utilities::to_bool(m_flags & Flags::NAME_INPUT) == true)
        {
            file_browser_area.y -= dev_gui.calculate_text_size(m_file_name_input.label).y + gui_style.frame_padding.y * 2;
        }

        m_file_browser.draw(dev_gui, file_browser_area);
        if (Vadon::Utilities::to_bool(m_flags & Flags::NAME_INPUT) == true)
        {
            dev_gui.draw_input_text(m_file_name_input);
        }

        if (dev_gui.draw_button(m_accept_button) == true)
        {
            result = Result::ACCEPTED;
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
}