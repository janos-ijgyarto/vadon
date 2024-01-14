#ifndef VADONAPP_UI_DEVELOPER_GUI_HPP
#define VADONAPP_UI_DEVELOPER_GUI_HPP
#include <VadonApp/UI/UIModule.hpp>
#include <VadonApp/UI/Developer/GUIElements.hpp>
#include <VadonApp/Platform/Event/Event.hpp>
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
namespace VadonApp::UI::Developer
{
	// Developer GUI, primarily based on ImGui
	class GUISystem : public UISystemBase<GUISystem>
	{
	public:
		enum class IOFlags
		{
			NONE = 0,
			MOUSE_CAPTURE = 1 << 0,
			KEYBOARD_CAPTURE = 1 << 1,
			TEXT_INPUT = 1 << 2
		};

		enum class TreeNodeFlags
		{
			NONE = 0,
			SELECTED = 1 << 0,
			FRAMED = 1 << 1,
			DEFAULT_OPEN = 1 << 5,
			OPEN_ON_DOUBLE_CLICK = 1 << 6,
			OPEN_ON_ARROW = 1 << 7,
			LEAF = 1 << 8,
			BULLET = 1 << 9
		};

		virtual ~GUISystem() {}

		virtual IOFlags get_io_flags() const = 0;

		// FIXME:
		// - Decouple from specific window
		// - Allow rendering to arbitrary target
		// - Just generate draw commands, have client code render it?
		virtual void dispatch_platform_events(const Platform::PlatformEventList& platform_events) = 0;

		virtual void start_frame() = 0; // Start the frame (must be called before any GUI objects are drawn)
		virtual void end_frame() = 0; // End the frame (must be called to prepare for rendering)

		virtual void render() = 0; // Render the last frame directly (NOTE: this function assumes the shader and render target are set)

		virtual void cache_frame(int32_t frame_index) = 0; // Store frame render data (useful for multithreaded contexts)
		virtual void swap_frame(int32_t source_index, int32_t target_index) = 0; // Swap cached frame data (allows creating a "ready buffer" for multithreaded environments)
		virtual void render_frame(int32_t frame_index) = 0; // Render from cached frame

		virtual void push_id(std::string_view string_id) = 0;
		virtual void push_id(const void* pointer_id) = 0;
		virtual void push_id(int32_t int_id) = 0;
		virtual void pop_id() = 0;

		virtual bool begin_window(Window& window) = 0;
		virtual void end_window() = 0;

		virtual bool is_window_focused() const = 0; // TODO: flags!
		virtual bool is_window_hovered() const = 0; // TODO: flags!
		
		virtual bool begin_child_window(const ChildWindow& window) = 0;
		virtual void end_child_window() = 0;

		virtual bool push_tree_node(std::string_view label, TreeNodeFlags flags = TreeNodeFlags::NONE) = 0;
		virtual bool push_tree_node(std::string_view id, std::string_view label, TreeNodeFlags flags = TreeNodeFlags::NONE) = 0;
		virtual bool push_tree_node(const void* id, std::string_view label, TreeNodeFlags flags = TreeNodeFlags::NONE) = 0;
		virtual void pop_tree_node() = 0;

		virtual bool draw_input_int(InputInt& input_int) = 0;
		virtual bool draw_input_int2(InputInt2& input_int) = 0;
		virtual bool draw_input_float(InputFloat& input_float) = 0;
		virtual bool draw_input_float2(InputFloat2& input_float) = 0;

		virtual bool draw_input_text(InputText& input_text) = 0;

		virtual bool draw_slider_int(SliderInt& slider) = 0;
		virtual bool draw_slider_int2(SliderInt2& slider) = 0;
		virtual bool draw_slider_float(SliderFloat& slider) = 0;
		virtual bool draw_slider_float2(SliderFloat2& slider) = 0;

		virtual bool draw_color3_picker(InputFloat3& color) = 0;
		
		virtual bool draw_button(const Button& button) = 0;

		virtual bool draw_checkbox(Checkbox& checkbox) = 0;

		virtual bool draw_list_box(ListBox& list_box) = 0;
		virtual bool draw_combo_box(ComboBox& combo_box) = 0;

		// NOTE: use in combination with add_text to enter the cell contents
		virtual bool begin_table(const Table& table) = 0;
		virtual void next_table_column() = 0;
		virtual void end_table() = 0;

		virtual void add_text(std::string_view text) = 0;
		virtual void add_text_unformatted(std::string_view text) = 0;

		virtual void same_line() = 0;
		virtual void set_scroll_x(float ratio = 0.5f) = 0;
		virtual void set_scroll_y(float ratio = 0.5f) = 0;

		virtual bool is_item_hovered() const = 0;
		virtual bool is_item_active() const = 0;
		virtual bool is_item_focused() const = 0;
		virtual bool is_item_clicked(Platform::MouseButton mouse_button = Platform::MouseButton::LEFT) const = 0;
		virtual bool is_item_toggled_open() const = 0;

		virtual bool is_key_down(Platform::KeyCode key) const = 0;
		virtual bool is_key_pressed(Platform::KeyCode key, bool repeat = true) const = 0;
		virtual bool is_key_released(Platform::KeyCode key) const = 0;
		 
		virtual bool is_mouse_down(Platform::MouseButton button) const = 0;
		virtual bool is_mouse_clicked(Platform::MouseButton button, bool repeat = false) const = 0;
		virtual bool is_mouse_released(Platform::MouseButton button) const = 0;
		virtual bool is_mouse_double_clicked(Platform::MouseButton button) const = 0;
	protected:
		GUISystem(Core::Application& application) : System(application) {}
	};
}
VADON_ENABLE_BITWISE_OPERATORS(VadonApp::UI::Developer::GUISystem::IOFlags)
VADON_ENABLE_BITWISE_OPERATORS(VadonApp::UI::Developer::GUISystem::TreeNodeFlags)
#endif