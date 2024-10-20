#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/UI/Developer/Dummy/GUI.hpp>

#include <VadonApp/Private/Core/Application.hpp>

#include <VadonApp/Private/UI/Developer/Dummy/GUIElements.hpp>

namespace VadonApp::Private::UI::Developer::Dummy
{
    GUISystem::GUISystem(VadonApp::Core::Application& application)
        : VadonApp::Private::UI::Developer::GUISystem(application)
    {

    }

    // TODO: implement some kind of "dummy GUI stack" so we can see what the app is trying to draw, even if the dev GUI is disabled?

    void GUISystem::start_frame()
    {
        // TODO!!!
    }

    void GUISystem::end_frame()
    {
        // TODO!!!
    }

    void GUISystem::render()
    {
        // TODO!!!
    }

    void GUISystem::cache_frame(int32_t /*frame_index*/)
    {
        // TODO!!!
    }

    void GUISystem::swap_frame(int32_t /*source_index*/, int32_t /*target_index*/)
    {
        // TODO!!!
    }

    void GUISystem::render_frame(int32_t /*frame_index*/)
    {
        // TODO!!!
    }

    bool GUISystem::initialize()
    {
        // TODO!!!
        log_message("Dummy developer GUI initialized successfully!\n");
        return true;
    }

    void GUISystem::shutdown()
    {
        // TODO!!!
        log_message("Dummy developer GUI shut down successfully!\n");
    }

    void GUISystem::push_id(std::string_view /*string_id*/)
    {
        // TODO
    }

    void GUISystem::push_id(const void* /*pointer_id*/)
    {
        // TODO
    }

    void GUISystem::push_id(int32_t /*int_id*/)
    {
        // TODO
    }

    void GUISystem::pop_id()
    {
        // TODO
    }

    GUISystem::ID GUISystem::get_id(std::string_view /*string_id*/)
    {
        // TODO
        return 0;
    }

    GUISystem::ID GUISystem::get_id(const void* /*pointer_id*/)
    {
        // TODO
        return 0;
    }

    void GUISystem::begin_disabled(bool /*disabled*/)
    {
        // TODO
    }

    void GUISystem::end_disabled()
    {
        // TODO
    }

    bool GUISystem::begin_window(Window& /*window*/)
    {
        // TODO!!!
        return true;
    }

    void GUISystem::end_window()
    {        
    }

    bool GUISystem::is_window_focused() const
    {
        return false;
    }

    bool GUISystem::is_window_hovered() const
    {
        return false;
    }

    bool GUISystem::begin_child_window(const ChildWindow& /*window*/)
    {
        // TODO!!!
        return true;
    }

    void GUISystem::end_child_window()
    {
    }

    void GUISystem::open_dialog(std::string_view /*id*/)
    {
        // TODO
    }

    void GUISystem::close_current_dialog()
    {
        // TODO
    }

    bool GUISystem::begin_modal_dialog(Window& /*dialog*/)
    {
        return false;
    }

    void GUISystem::end_dialog()
    {
        // TODO
    }

    bool GUISystem::begin_main_menu_bar()
    {
        // TODO
        return false;
    }

    void GUISystem::end_main_menu_bar()
    {
        // TODO
    }

    bool GUISystem::begin_menu_bar()
    {
        // TODO
        return false;
    }

    void GUISystem::end_menu_bar()
    {
        // TODO
    }

    bool GUISystem::begin_menu(std::string_view /*label*/, bool /*enabled*/)
    {
        // TODO
        return false;
    }

    void GUISystem::end_menu()
    {
        // TODO
    }

    bool GUISystem::add_menu_item(const MenuItem& /*menu_item*/)
    {
        // TODO
        return false;
    }

    bool GUISystem::push_tree_node(std::string_view /*label*/, TreeNodeFlags /*flags*/)
    {
        return false;
    }

    bool GUISystem::push_tree_node(std::string_view /*id*/, std::string_view /*label*/, TreeNodeFlags /*flags*/)
    {
        return false;
    }

    bool GUISystem::push_tree_node(const void* /*id*/, std::string_view /*label*/, TreeNodeFlags /*flags*/)
    {
        return false;
    }

    void GUISystem::pop_tree_node()
    {        
    }

    bool GUISystem::draw_input_int(InputInt& /*input_int*/)
    {
        return false;
    }

    bool GUISystem::draw_input_int2(InputInt2& /*input_int*/)
    {
        return false;
    }

    bool GUISystem::draw_input_float(InputFloat& /*input_float*/)
    {
        return false;
    }

    bool GUISystem::draw_input_float2(InputFloat2& /*input_float*/)
    {
        return false;
    }

    bool GUISystem::draw_input_text(InputText& /*input_text*/)
    {
        // TODO!!!
        return true;
    }

    bool GUISystem::draw_slider_int(SliderInt& /*slider*/)
    {
        // TODO!!!
        return false;
    }

    bool GUISystem::draw_slider_int2(SliderInt2& /*slider*/)
    {
        // TODO!!!
        return false;
    }

    bool GUISystem::draw_slider_float(SliderFloat& /*slider*/)
    {
        // TODO!!!
        return false;
    }

    bool GUISystem::draw_slider_float2(SliderFloat2& /*slider*/)
    {
        // TODO!!!
        return false;
    }

    bool GUISystem::draw_color3_picker(InputFloat3& /*color*/)
    {
        return false;
    }

    bool GUISystem::draw_button(const Button& /*button*/)
    {
        // TODO!!!
        return false;
    }

    bool GUISystem::draw_checkbox(Checkbox& /*checkbox*/)
    {
        // TODO!!!
        return false;
    }

    bool GUISystem::draw_list_box(ListBox& /*list_box*/)
    {
        return false;
    }

    bool GUISystem::draw_combo_box(ComboBox& /*combo_box*/)
    {
        return false;
    }

    bool GUISystem::begin_table(const Table& /*table*/)
    {
        return false;
    }

    void GUISystem::next_table_column()
    {
        // TODO
    }

    void GUISystem::end_table()
    {
        // TODO
    }

    void GUISystem::add_separator()
    {
        // TODO
    }

    void GUISystem::add_text(std::string_view /*text*/)
    {
        // TODO
    }

    void GUISystem::add_text_unformatted(std::string_view /*text*/)
    {
        // TODO
    }

    void GUISystem::add_separator_text(std::string_view /*text*/)
    {
        // TODO
    }

    void GUISystem::same_line()
    {
        // TODO
    }

    void GUISystem::set_scroll_x(float /*ratio*/)
    {
        // TODO
    }

    void GUISystem::set_scroll_y(float /*ratio*/)
    {
        // TODO
    }

    bool GUISystem::is_item_hovered() const
    {
        return false;
    }

    bool GUISystem::is_item_active() const
    {
        return false;
    }

    bool GUISystem::is_item_focused() const
    {
        return false;
    }

    bool GUISystem::is_item_clicked(VadonApp::Platform::MouseButton /*mouse_button*/) const
    {
        return false;
    }

    bool GUISystem::is_item_toggled_open() const
    {
        return false;
    }

    bool GUISystem::is_key_down(VadonApp::Platform::KeyCode /*key*/) const
    {
        return false;
    }

    bool GUISystem::is_key_pressed(VadonApp::Platform::KeyCode /*key*/, bool /*repeat*/) const
    {
        return false;
    }

    bool GUISystem::is_key_released(VadonApp::Platform::KeyCode /*key*/) const
    {
        return false;
    }

    bool GUISystem::is_mouse_down(VadonApp::Platform::MouseButton /*button*/) const
    {
        return false;
    }

    bool GUISystem::is_mouse_clicked(VadonApp::Platform::MouseButton /*button*/, bool /*repeat*/) const
    {
        return false;
    }

    bool GUISystem::is_mouse_released(VadonApp::Platform::MouseButton /*button*/) const
    {
        return false;
    }

    bool GUISystem::is_mouse_double_clicked(VadonApp::Platform::MouseButton /*button*/) const
    {
        return false;
    }
}