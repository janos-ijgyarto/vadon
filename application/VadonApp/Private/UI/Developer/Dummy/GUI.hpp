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

		void dispatch_platform_events(const VadonApp::Platform::PlatformEventList& platform_events) override;

		void start_frame() override;
		void end_frame() override;

		void render() override;

		void cache_frame(int32_t frame_index) override;
		void swap_frame(int32_t source_index, int32_t target_index) override;
		void render_frame(int32_t frame_index) override;

		bool initialize() override;
		void shutdown() override;

		bool begin_window(Window& window) override;
		void end_window() override;

		bool begin_child_window(const ChildWindow& window) override;
		void end_child_window() override;

		bool push_tree_node(std::string_view label) override;
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

		void add_text(std::string_view text) override;
		void add_text_unformatted(std::string_view text) override;

		void same_line() override;
		void set_scroll_x(float ratio = 0.5f) override;
		void set_scroll_y(float ratio = 0.5f) override;
	};
}
#endif