#ifndef VADONAPP_PRIVATE_UI_DEVELOPER_IMGUI_GUI_HPP
#define VADONAPP_PRIVATE_UI_DEVELOPER_IMGUI_GUI_HPP
#include <VadonApp/Private/UI/Developer/GUI.hpp>
#include <VadonApp/Private/UI/Developer/ImGui/GUIElements.hpp>

#include <VadonApp/Platform/Event/Event.hpp>

#include <Vadon/Render/GraphicsAPI/Buffer/Buffer.hpp>
#include <Vadon/Render/GraphicsAPI/Pipeline/Pipeline.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/SRV.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/Texture.hpp>
namespace VadonApp::Private::UI::Developer::ImGUI
{
	using GUIStyle = VadonApp::UI::Developer::GUIStyle;

	class GUISystem final : public VadonApp::Private::UI::Developer::GUISystem
	{
	public:
		GUISystem(VadonApp::Core::Application& application);
		~GUISystem();

		bool initialize() override;
		void shutdown() override;

		IOFlags get_io_flags() const override;
		GUIStyle get_style() const override;

		void set_platform_window(VadonApp::Platform::WindowHandle window_handle) override;

		void start_frame() override;
		void end_frame() override;

		void render() override;

		void push_id(std::string_view string_id) override;
		void push_id(const void* pointer_id) override;
		void push_id(int32_t int_id) override;
		void pop_id() override;

		ID get_id(std::string_view string_id) override;
		ID get_id(const void* pointer_id) override;

		void begin_disabled(bool disabled) override;
		void end_disabled() override;

		Vadon::Utilities::Vector2 get_available_content_region() const override;
		Vadon::Utilities::Vector2 calculate_text_size(std::string_view text, std::string_view text_end = "", bool hide_after_double_hash = false, float wrap_width = -1.0f) const override;

		void push_item_width(float item_width) override;
		void pop_item_width() override;
		void set_next_item_width(float item_width) override;

		bool begin_window(Window& window) override;
		void end_window() override;

		bool is_window_focused() const override;
		bool is_window_hovered() const override;

		bool begin_child_window(const ChildWindow& window) override;
		void end_child_window() override;

		bool begin_popup(Window& popup) override;
		bool begin_popup_modal(Window& popup) override;
		void end_popup() override;

		void open_popup(std::string_view id) override;
		void close_current_popup() override;

		bool begin_popup_context_item(std::string_view id) override;

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
		bool draw_input_float3(InputFloat3& input_float) override;

		bool draw_input_text(InputText& input_text) override;

		bool draw_slider_int(SliderInt& slider) override;
		bool draw_slider_int2(SliderInt2& slider) override;
		bool draw_slider_float(SliderFloat& slider) override;
		bool draw_slider_float2(SliderFloat2& slider) override;

		bool draw_color_edit(ColorEdit& color) override;

		bool draw_button(const Button& button) override;

		bool draw_checkbox(Checkbox& checkbox) override;

		bool draw_list_box(ListBox& list_box, bool* double_clicked = nullptr) override;
		bool draw_combo_box(ComboBox& combo_box) override;

		bool begin_table(const Table& table) override;
		void next_table_column() override;
		void end_table() override;

		void add_separator() override;
		void add_text(std::string_view text) override;
		void add_text_unformatted(std::string_view text) override;
		void add_text_wrapped(std::string_view text) override;
		void add_separator_text(std::string_view text) override;

		void draw_clipped_text_list(const TextBuffer& text_buffer) override;

		void set_item_tooltip(std::string_view tooltip_text) override;
		
		void same_line() override;
		void set_scroll_x(float ratio = 0.5f) override;
		void set_scroll_y(float ratio = 0.5f) override;

		bool is_item_hovered() const override;
		bool is_item_active() const override;
		bool is_item_focused() const override;
		bool is_item_clicked(VadonApp::Platform::MouseButton mouse_button = VadonApp::Platform::MouseButton::LEFT) const override;
		bool is_item_toggled_open() const override;
		bool is_item_edited() const override;

		bool is_key_down(VadonApp::Platform::KeyCode key) const override;
		bool is_key_pressed(VadonApp::Platform::KeyCode key, bool repeat = true) const override;
		bool is_key_released(VadonApp::Platform::KeyCode key) const override;

		bool is_mouse_down(VadonApp::Platform::MouseButton button) const override;
		bool is_mouse_clicked(VadonApp::Platform::MouseButton button, bool repeat = false) const override;
		bool is_mouse_released(VadonApp::Platform::MouseButton button) const override;
		bool is_mouse_double_clicked(VadonApp::Platform::MouseButton button) const override;
	private:
		struct PlatformUserData
		{
			VadonApp::Platform::WindowHandle window_handle;

			uint64_t performance_frequency = 0;
			uint64_t current_time = 0;
			int mouse_buttons_down = 0;
			int pending_mouse_leave_frame = 0;
			bool mouse_global_state = false;
		};

		struct Buffer
		{
			Vadon::Render::BufferHandle buffer_handle;
			int32_t capacity = 0;

			bool is_valid(int32_t new_capacity) const;
		};

		bool init_platform();
		bool init_renderer();

		void update_platform();
		void process_platform_events(const VadonApp::Platform::PlatformEventList& platform_events);

		void update_mouse_data();
		void update_mouse_cursor();
		void update_gamepads();

		void update_buffers(int32_t vertex_count, int32_t index_count);

		static const char* get_clipboard_text(void* user_data);
		static void set_clipboard_text(void* user_data, const char* text);

		PlatformUserData m_platform_data;

		Vadon::Render::PipelineState m_pipeline_state;

		Vadon::Render::ShaderHandle m_vertex_shader;
		Vadon::Render::ShaderHandle m_pixel_shader;
		Vadon::Render::VertexLayoutHandle m_vertex_layout;

		Buffer m_vertex_buffer;
		Buffer m_index_buffer;
		Vadon::Render::BufferHandle m_constant_buffer;

		Vadon::Render::TextureHandle m_fonts_texture;
		Vadon::Render::TextureSamplerHandle m_sampler;

		std::unordered_map<size_t, Vadon::Render::SRVHandle> m_texture_lookup;
		size_t m_texture_counter;

		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif