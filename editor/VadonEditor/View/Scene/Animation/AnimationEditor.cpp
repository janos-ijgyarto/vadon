#include <VadonEditor/View/Scene/Animation/AnimationEditor.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/View/ViewSystem.hpp>

#include <Vadon/Scene/Animation/AnimationSystem.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/TypeInfo/TypeErasure.hpp>

#include <VadonApp/UI/Developer/IconsFontAwesome5.h>

#include <algorithm>

namespace
{
	constexpr const char* c_animation_channel_type_labels[static_cast<size_t>(Vadon::Scene::AnimationChannelType::TYPE_COUNT)] = {
		"int",
		"uint32_t",
		"float",
		"Vector2",
		"Vector2i",
		"Vector3",
		"Vector3i",
		"Vector4",
		"ColorRGBA"
	};

	constexpr const char* get_animation_channel_type_label(Vadon::Scene::AnimationChannelType channel_type)
	{
		return c_animation_channel_type_labels[static_cast<size_t>(channel_type)];
	}
}

namespace VadonEditor::View
{
	UI::Developer::Dialog::Result AnimationAddChannelDialog::internal_draw(VadonApp::UI::Developer::GUISystem& dev_gui)
	{
		Result result = Result::NONE;

		dev_gui.draw_combo_box(m_data_type_combo);

		const bool valid = is_valid();
		
		if (valid == false)
		{
			dev_gui.begin_disabled(true);
		}

		if (dev_gui.draw_button(m_create_button) == true)
		{
			result = Result::ACCEPTED;
			close();
		}

		if (valid == false)
		{
			dev_gui.end_disabled();
		}
		dev_gui.same_line();
		if (dev_gui.draw_button(m_cancel_button) == true)
		{
			result = Result::CANCELLED;
			close();
		}

		return result;
	}

	AnimationAddChannelDialog::AnimationAddChannelDialog()
		: UI::Developer::Dialog("Add Animation Channel")
	{
		m_data_type_combo.label = "Data type";

		for (int32_t channel_type_index = 0; channel_type_index < static_cast<int32_t>(Vadon::Scene::AnimationChannelType::TYPE_COUNT); ++channel_type_index)
		{
			m_data_type_combo.items.push_back(get_animation_channel_type_label(Vadon::Utilities::to_enum<Vadon::Scene::AnimationChannelType>(channel_type_index)));
		}

		m_data_type_combo.selected_item = 0;

		m_create_button.label = "Create";
		m_cancel_button.label = "Cancel";
	}

	bool AnimationAddChannelDialog::is_valid() const
	{
		if (m_data_type_combo.has_valid_selection() == false)
		{
			return false;
		}

		return true;
	}

	AnimationEditor::KeyframeEditor::KeyframeEditor(AnimationEditor& parent)
		: parent_window(parent)
	{
		child_window.id = "##keyframe_editor";
		child_window.border = true;
	}

	void AnimationEditor::KeyframeEditor::update_selection(const Vadon::Math::Vector2i& frame_coords)
	{
		if (selected_frame_coords == frame_coords)
		{
			return;
		}

		selected_frame_coords = frame_coords;
		property_editor.reset();

		if (frame_coords == Vadon::Math::Vector2i{ -1, -1 })
		{
			return;
		}

		const Channel& view_channel = parent_window.m_channels[frame_coords.y];

		const KeyframeTableCell& keyframe_cell = view_channel.cells[frame_coords.x];
		if (keyframe_cell.value.index() == 0)
		{
			return;
		}

		Vadon::Utilities::Property keyframe_property_data;
		keyframe_property_data.data_type = Vadon::Scene::AnimationChannel::get_data_type_id(view_channel.data_type);
		keyframe_property_data.name = "Keyframe value";
		keyframe_property_data.value = keyframe_cell.value;

		property_editor = PropertyEditor::create_property_editor(parent_window.m_editor, keyframe_property_data);
	}

	bool AnimationEditor::KeyframeEditor::draw(VadonApp::UI::Developer::GUISystem& dev_gui)
	{
		bool modified = false;

		const Vadon::Math::Vector2 window_size = dev_gui.get_available_content_region();
		child_window.size.x = window_size.x;
		child_window.size.y = window_size.y * 0.2f;

		if (dev_gui.begin_child_window(child_window) == true)
		{
			dev_gui.add_text("Keyframe Editor");
			if (property_editor != nullptr)
			{
				if (property_editor->render(dev_gui) == true)
				{
					// TODO: make proper use of this and only update once explicitly requested (and only the modified elements)?
					property_editor->clear_modified();

					const Vadon::Utilities::Property& property_data = property_editor->get_property();

					Channel& view_channel = parent_window.m_channels[selected_frame_coords.y];
					KeyframeTableCell& keyframe_cell = view_channel.cells[selected_frame_coords.x];

					keyframe_cell.value = property_data.value;

					modified = true;
				}
			}
		}
		dev_gui.end_child_window();

		return modified;
	}

	AnimationEditor::Channel::Channel()
	{
		tag_input.label = "Tag";
	}

	AnimationEditor::AnimationEditor(Core::Editor& editor)
		: m_editor(editor)
		, m_animation_data(nullptr)
		, m_keyframe_editor(*this)
	{
		m_window.title = "Animation Editor";
		m_window.open = false;

		m_frame_count_input.label = "Frame count";
		m_frame_count_input.input = 0;
		m_frame_count_input.flags = UI::Developer::InputFlags::ENTER_RETURNS_TRUE;

		m_timeline_table.label = "Timeline";

		m_add_channel_button.label = "Add channel";

		m_add_keyframe_menu_item.label = "Add keyframe";
		m_add_keyframe_menu_item.enabled = true;

		m_remove_keyframe_menu_item.label = "Remove keyframe";
		m_remove_keyframe_menu_item.enabled = true;

		m_remove_channel_menu_item.label = "Remove channel";
		m_remove_channel_menu_item.enabled = true;
	}

	void AnimationEditor::draw(UI::Developer::GUISystem& dev_gui)
	{
		update_animation_resource();

		if (m_animation_data == nullptr)
		{
			return;
		}

		if (m_add_channel_dialog.draw(dev_gui) == UI::Developer::Dialog::Result::ACCEPTED)
		{
			add_channel();
		}

		if (dev_gui.begin_window(m_window) == true)
		{
			bool modified = m_keyframe_editor.draw(dev_gui);

			if (dev_gui.draw_input_int(m_frame_count_input) == true)
			{
				m_frame_count_input.input = std::max(m_frame_count_input.input, 0);
				frame_count_changed();

				modified = true;
			}

			if (dev_gui.draw_button(m_add_channel_button) == true)
			{
				m_add_channel_dialog.open();

				modified = true;
			}

			if (dev_gui.begin_table(m_timeline_table) == true)
			{
				// First draw the header
				for (int32_t column_index = 0; column_index < m_timeline_table.column_count; ++column_index)
				{
					dev_gui.next_table_column();
					if (column_index > 0)
					{
						dev_gui.add_text(std::format("{}", column_index - 1));
					}
					else
					{
						dev_gui.add_text("Channels");
					}
				}

				for (size_t channel_index = 0; channel_index < m_channels.size();)
				{
					Channel& channel = m_channels[channel_index];

					bool channel_removed = false;

					for (int32_t column_index = 0; (column_index < m_timeline_table.column_count) && (channel_removed == false); ++column_index)
					{
						const int32_t dev_gui_push_id = (static_cast<int32_t>(channel_index) * m_timeline_table.column_count) + column_index;
						dev_gui.next_table_column();
						if (column_index > 0)
						{
							const int32_t keyframe_index = column_index - 1;
							const Vadon::Math::Vector2i frame_coords = { keyframe_index, static_cast<int32_t>(channel_index) };
							if (channel.cells[keyframe_index].value.index() != 0)
							{
								// Keyframe
								dev_gui.push_id(dev_gui_push_id);
								if (dev_gui.draw_selectable(ICON_FA_DOT_CIRCLE, m_keyframe_editor.selected_frame_coords == frame_coords))
								{
									m_keyframe_editor.update_selection(frame_coords);
								}
								if (dev_gui.begin_popup_context_item("##keyframe_popup") == true)
								{
									if (dev_gui.add_menu_item(m_remove_keyframe_menu_item) == true)
									{
										channel.cells[keyframe_index].value = Vadon::Utilities::Variant();
										modified = true;
									}
									dev_gui.end_popup();
								}
								dev_gui.pop_id();
							}
							else
							{
								// Empty cell
								dev_gui.add_text(ICON_FA_CIRCLE);
								dev_gui.push_id(dev_gui_push_id);
								if (dev_gui.begin_popup_context_item("##keyframe_popup") == true)
								{
									if (dev_gui.add_menu_item(m_add_keyframe_menu_item) == true)
									{
										const Vadon::Utilities::ErasedDataTypeID type_id = Vadon::Scene::AnimationChannel::get_data_type_id(channel.data_type);
										channel.cells[keyframe_index].value = Vadon::Utilities::get_erased_type_default_value(type_id);

										modified = true;
									}
									dev_gui.end_popup();
								}
								dev_gui.pop_id();
							}
						}
						else
						{
							dev_gui.add_text(std::format("Channel_{} ({})", channel_index, get_animation_channel_type_label(channel.data_type)));
							dev_gui.push_id(dev_gui_push_id);
							if (dev_gui.begin_popup_context_item("##remove_channel_popup") == true)
							{
								if (dev_gui.add_menu_item(m_remove_channel_menu_item) == true)
								{
									m_channels.erase(m_channels.begin() + channel_index);

									modified = true;
									channel_removed = true;
								}
								dev_gui.end_popup();
							}
							if (channel_removed != true)
							{
								if (dev_gui.draw_input_text(channel.tag_input) == true)
								{
									modified = true;
								}
							}
							dev_gui.pop_id();
						}
					}

					if (channel_removed == true)
					{
						continue;
					}

					++channel_index;
				}
				dev_gui.end_table();
			}

			if (modified == true)
			{
				// FIXME: this means even the slightest change will immediately re-export the entire animation
				// Could optimize this somehow?
				export_animation_data();
			}
		}
		dev_gui.end_window();
	}

	void AnimationEditor::update_animation_resource()
	{
		ViewSystem& view_system = m_editor.get_system<ViewSystem>();
		ViewModel& view_model = view_system.get_view_model();

		Model::Resource* active_resource = view_model.get_active_resource();
		if (active_resource == nullptr)
		{
			reset_animation_data();
			return;
		}

		if (active_resource == m_anim_resource)
		{
			// Animation already open
			return;
		}

		const Vadon::Scene::ResourceInfo& resource_info = active_resource->get_info();
		if (resource_info.type_id != Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::Animation>())
		{
			reset_animation_data();
			return;
		}

		// Reset widgets
		m_keyframe_editor.clear_selection();
		m_channels.clear();
		m_frame_count_input.input = 0;
				
		Vadon::Scene::AnimationSystem& animation_system = m_editor.get_engine_core().get_system<Vadon::Scene::AnimationSystem>();
		Vadon::Scene::AnimationHandle animation_handle = animation_system.load_animation(Vadon::Scene::AnimationID::from_resource_id(resource_info.id));
		if (animation_handle.is_valid() == false)
		{
			// TODO: log error?
			return;
		}

		m_animation_data = &animation_system.get_animation_data(animation_handle);
		m_channels.reserve(m_animation_data->channels.size());

		for (const Vadon::Scene::AnimationChannel& current_channel_data : m_animation_data->channels)
		{			
			Channel& view_channel = m_channels.emplace_back();
			view_channel.tag_input.input = current_channel_data.tag;
			view_channel.data_type = current_channel_data.data_type;

			view_channel.cells.resize(m_animation_data->frame_count);

			for (int32_t keyframe_index = 0; keyframe_index < current_channel_data.keyframe_range.count; ++keyframe_index)
			{
				const Vadon::Scene::AnimationKeyframe& current_keyframe = m_animation_data->keyframe_data[current_channel_data.keyframe_range.offset + keyframe_index];
				view_channel.cells[current_keyframe.frame_index].value = current_keyframe.value;
			}
		}

		m_frame_count_input.input = m_animation_data->frame_count;
		m_timeline_table.column_count = m_animation_data->frame_count + 1;
		m_window.open = true;

		m_anim_resource = active_resource;
	}

	void AnimationEditor::reset_animation_data()
	{
		if (m_anim_resource != nullptr)
		{
			m_anim_resource = nullptr;
			m_animation_data = nullptr;
			m_window.open = false;
		}
	}

	void AnimationEditor::add_channel()
	{
		m_keyframe_editor.clear_selection();

		Channel& view_channel = m_channels.emplace_back();
		view_channel.data_type = Vadon::Utilities::to_enum<Vadon::Scene::AnimationChannelType>(m_add_channel_dialog.m_data_type_combo.selected_item);

		view_channel.cells.resize(m_frame_count_input.input);
	}

	void AnimationEditor::frame_count_changed()
	{
		m_keyframe_editor.clear_selection();

		const int32_t frame_count = m_frame_count_input.input;

		for (Channel& current_channel : m_channels)
		{
			current_channel.cells.resize(frame_count);
		}

		m_timeline_table.column_count = frame_count + 1;
	}

	void AnimationEditor::export_animation_data()
	{
		Vadon::Scene::AnimationData animation_data;

		animation_data.frame_count = m_frame_count_input.input;
		animation_data.channels.reserve(m_channels.size());

		for (const Channel& current_channel : m_channels)
		{
			Vadon::Scene::AnimationChannel& anim_channel_data = animation_data.channels.emplace_back();

			anim_channel_data.tag = current_channel.tag_input.input;
			anim_channel_data.keyframe_range.offset = static_cast<int32_t>(animation_data.keyframe_data.size());
			anim_channel_data.data_type = current_channel.data_type;

			for (int32_t current_frame_index = 0; current_frame_index < current_channel.cells.size(); ++current_frame_index)
			{
				const KeyframeTableCell& current_cell = current_channel.cells[current_frame_index];
				if (current_cell.value.index() == 0)
				{
					continue;
				}

				animation_data.keyframe_data.push_back(Vadon::Scene::AnimationKeyframe{ .frame_index = current_frame_index, .value = current_cell.value });

				++anim_channel_data.keyframe_range.count;
			}
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_editor.get_engine_core();
		Vadon::Scene::AnimationSystem& animation_system = engine_core.get_system<Vadon::Scene::AnimationSystem>();
		Vadon::Scene::AnimationHandle animation_handle = animation_system.load_animation(Vadon::Scene::AnimationID::from_resource_id(m_anim_resource->get_info().id));

		animation_system.set_animation_data(animation_handle, animation_data);

		// Notify the resource so we can save the changes
		m_anim_resource->notify_modified();
	}
}