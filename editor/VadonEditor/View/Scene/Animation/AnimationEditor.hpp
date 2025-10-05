#ifndef VADONEDITOR_VIEW_SCENE_ANIMATION_ANIMATIONEDITOR_HPP
#define VADONEDITOR_VIEW_SCENE_ANIMATION_ANIMATIONEDITOR_HPP
#include <VadonEditor/UI/Developer/Widgets.hpp>
#include <VadonEditor/View/Scene/Property/Property.hpp>
#include <Vadon/Scene/Animation/Animation.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Resource;
}
namespace VadonEditor::View
{
	class AnimationAddChannelDialog : public UI::Developer::Dialog
	{
	protected:
		Dialog::Result internal_draw(VadonApp::UI::Developer::GUISystem& dev_gui) override;
	private:
		AnimationAddChannelDialog();

		bool is_valid() const;

		UI::Developer::ComboBox m_data_type_combo;
		UI::Developer::Button m_create_button;
		UI::Developer::Button m_cancel_button;

		friend class AnimationEditor;
	};

	class AnimationEditor
	{
	private:
		struct KeyframeEditor
		{
			AnimationEditor& parent_window;

			UI::Developer::ChildWindow child_window;
			PropertyEditor::Instance property_editor;
			Vadon::Math::Vector2i selected_frame_coords = { -1, -1 };

			KeyframeEditor(AnimationEditor& parent);

			void update_selection(const Vadon::Math::Vector2i& frame_coords);
			void clear_selection() { update_selection(Vadon::Math::Vector2i{ -1, -1 }); }
			
			// NOTE: if it returns true, the value was edited
			bool draw(VadonApp::UI::Developer::GUISystem& dev_gui);
		};

		struct KeyframeTableCell
		{
			Vadon::Utilities::Variant value;
		};

		struct Channel
		{
			UI::Developer::InputText tag_input;
			Vadon::Scene::AnimationChannelType data_type;
			std::vector<KeyframeTableCell> cells;

			Channel();
		};

		AnimationEditor(Core::Editor& editor);

		void draw(UI::Developer::GUISystem& dev_gui);

		void update_animation_resource();
		void reset_animation_data();

		void add_channel();
		void frame_count_changed();

		void export_animation_data();

		Core::Editor& m_editor;

		UI::Developer::Window m_window;

		VadonEditor::Model::Resource* m_anim_resource;
		const Vadon::Scene::AnimationData* m_animation_data;

		KeyframeEditor m_keyframe_editor;
		UI::Developer::Table m_timeline_table;
		std::vector<Channel> m_channels;
		UI::Developer::InputInt m_frame_count_input;

		UI::Developer::Button m_add_channel_button;
		AnimationAddChannelDialog m_add_channel_dialog;

		UI::Developer::MenuItem m_add_keyframe_menu_item;
		UI::Developer::MenuItem m_remove_keyframe_menu_item;
		UI::Developer::MenuItem m_remove_channel_menu_item;

		friend class MainWindow;
	};
}
#endif