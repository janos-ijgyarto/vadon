#ifndef VADONAPP_PRIVATE_UI_DEVELOPER_DUMMY_GUI_HPP
#define VADONAPP_PRIVATE_UI_DEVELOPER_DUMMY_GUI_HPP
#include <VadonApp/Private/UI/Developer/GUI.hpp>
#include <VadonApp/Private/UI/Developer/Dummy/GUIElements.hpp>
namespace VadonApp::Private::UI::Developer::Dummy
{
	class GUISystem final : public VadonApp::Private::UI::Developer::GUISystem
	{
	public:
		GUISystem(VadonApp::Core::Application& application);

		IOFlags get_io_flags() const override { return IOFlags::NONE; }

		void start_frame() override;
		void end_frame() override;

		void render() override;

		void cache_frame(int32_t frame_index) override;
		void swap_frame(int32_t source_index, int32_t target_index) override;
		void render_frame(int32_t frame_index) override;

		bool initialize() override;
		void shutdown() override;

		void push_id(std::string_view string_id) override;
		void push_id(const void* pointer_id) override;
		void push_id(int32_t int_id) override;
		void pop_id() override;

		ID get_id(std::string_view string_id) override;
		ID get_id(const void* pointer_id) override;

		void begin_disabled(bool disabled) override;
		void end_disabled() override;

		bool begin_window(Window& window) override;
		void end_window() override;

		bool is_window_focused() const override;
		bool is_window_hovered() const override;

		bool begin_child_window(const ChildWindow& window) override;
		void end_child_window() override;

		void open_dialog(std::string_view id) override;
		void close_current_dialog() override;

		bool begin_modal_dialog(Window& dialog) override;
		void end_dialog() override;

		bool begin_main_menu_bar() override;
		void end_main_menu_bar() override;

		bool begin_menu_bar() override;
		void end_menu_bar() override;

		bool begin_menu(std::string_view label, bool enabled) override;
		void end_menu() override;

		bool add_menu_item(const MenuItem& menu_item) override;

		bool push_tree_node(std::string_view label, TreeNodeFlags flags = TreeNodeFlags::NONE) override;
		bool push_tree_node(std::string_view id, std::string_view label, TreeNodeFlags flags = TreeNodeFlags::NONE) override;
		bool push_tree_node(const void* id, std::string_view label, TreeNodeFlags flags = TreeNodeFlags::NONE) override;
		void pop_tree_node() override;

		bool draw_input_int(InputInt& input_int) override;
		bool draw_input_int2(InputInt2& input_int) override;
		bool draw_input_float(InputFloat& input_float) override;
		bool draw_input_float2(InputFloat2& input_float) override;

		bool draw_input_text(InputText& input_text) override;

		bool draw_slider_int(SliderInt& slider) override;
		bool draw_slider_int2(SliderInt2& slider) override;
		bool draw_slider_float(SliderFloat& slider) override;
		bool draw_slider_float2(SliderFloat2& slider) override;

		bool draw_color3_picker(InputFloat3& color) override;

		bool draw_button(const Button& button) override;

		bool draw_checkbox(Checkbox& checkbox) override;

		bool draw_list_box(ListBox& list_box) override;
		bool draw_combo_box(ComboBox& combo_box) override;

		bool begin_table(const Table& table) override;
		void next_table_column() override;
		void end_table() override;

		void add_separator() override;
		void add_text(std::string_view text) override;
		void add_text_unformatted(std::string_view text) override;
		void add_separator_text(std::string_view text) override;

		void same_line() override;
		void set_scroll_x(float ratio = 0.5f) override;
		void set_scroll_y(float ratio = 0.5f) override;

		bool is_item_hovered() const override;
		bool is_item_active() const override;
		bool is_item_focused() const override;
		bool is_item_clicked(VadonApp::Platform::MouseButton mouse_button = VadonApp::Platform::MouseButton::LEFT) const override;
		bool is_item_toggled_open() const override;

		bool is_key_down(VadonApp::Platform::KeyCode key) const override;
		bool is_key_pressed(VadonApp::Platform::KeyCode key, bool repeat = true) const override;
		bool is_key_released(VadonApp::Platform::KeyCode key) const override;

		bool is_mouse_down(VadonApp::Platform::MouseButton button) const override;
		bool is_mouse_clicked(VadonApp::Platform::MouseButton button, bool repeat = false) const override;
		bool is_mouse_released(VadonApp::Platform::MouseButton button) const override;
		bool is_mouse_double_clicked(VadonApp::Platform::MouseButton button) const override;
	};
}
#endif