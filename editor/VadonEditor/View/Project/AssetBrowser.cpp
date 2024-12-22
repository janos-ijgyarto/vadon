#include <VadonEditor/View/Project/AssetBrowser.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>
#include <VadonEditor/Core/Project/Asset/AssetLibrary.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/View/ViewSystem.hpp>

#include <VadonApp/UI/Developer/IconsFontAwesome5.h>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>

#include <filesystem>
#include <format>

namespace
{
	void get_asset_node_label(const VadonEditor::Core::AssetNode& node, std::string& label)
	{
		const VadonEditor::Core::AssetInfo& info = node.get_info();
		switch (info.type)
		{
		case VadonEditor::Core::AssetType::NONE:
			label = ICON_FA_FOLDER + (" " + info.name);
			break;
		case VadonEditor::Core::AssetType::RESOURCE:
			label = ICON_FA_FILE_ALT + (" " + info.name);
			break;
		case VadonEditor::Core::AssetType::SCENE:
			label = ICON_FA_FILM + (" " + info.name);
			break;
		}
	}
}

namespace VadonEditor::View
{
	void CreateResourceDialog::on_open()
	{
		m_resource_types = Vadon::Utilities::TypeRegistry::get_subclass_list(Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::Resource>());
		m_resource_type_combo.deselect();
		m_resource_type_combo.items.clear();

		for (Vadon::Utilities::TypeID current_resource_type : m_resource_types)
		{
			const Vadon::Utilities::TypeInfo current_type_info = Vadon::Utilities::TypeRegistry::get_type_info(current_resource_type);
			m_resource_type_combo.items.push_back(current_type_info.name);
		}

		m_resource_type_combo.selected_item = 0;
	}

	CreateResourceDialog::Result CreateResourceDialog::internal_draw(UI::Developer::GUISystem& dev_gui)
	{
		Result result = Result::NONE;

		dev_gui.draw_combo_box(m_resource_type_combo);

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

	Vadon::Utilities::TypeID CreateResourceDialog::get_selected_resource_type() const
	{
		return m_resource_types[m_resource_type_combo.selected_item];
	}

	CreateResourceDialog::CreateResourceDialog(Core::Editor& editor)
		: Dialog("Create New Resource")
		, m_editor(editor)
	{
		m_resource_type_combo.label = "Resource types";

		m_accept_button.label = "Create";
		m_cancel_button.label = "Cancel";
	}

	AssetBrowser::AssetBrowser(Core::Editor& editor)
		: m_editor(editor)
		, m_create_resource_dialog(editor)
		, m_save_file_dialog("Save File", UI::Developer::FileBrowserDialog::Flags::NAME_INPUT)
	{
		m_window.title = "Asset Browser";
		m_window.open = false;

		m_asset_tree.id = "AssetTree";

		m_save_file_dialog.set_accept_label("Save");

		{
			UI::Developer::MenuItem& create_resource_menu_item = m_create_asset_context_menus[Vadon::Utilities::to_integral(CreateMenuOption::RESOURCE)];
			create_resource_menu_item.label = "Create Resource";
		}
		{
			UI::Developer::MenuItem& create_scene_menu_item = m_create_asset_context_menus[Vadon::Utilities::to_integral(CreateMenuOption::SCENE)];
			create_scene_menu_item.label = "Create Scene";
		}
	}

	void AssetBrowser::draw(UI::Developer::GUISystem& dev_gui)
	{
		if (dev_gui.begin_window(m_window) == true)
		{
			if (m_save_file_dialog.draw(dev_gui) == UI::Developer::Dialog::Result::ACCEPTED)
			{
				save_asset();
			}
			if (m_create_resource_dialog.draw(dev_gui) == UI::Developer::Dialog::Result::ACCEPTED)
			{
				create_resource();
			}

			// Make tree fill available content region
			m_asset_tree.size = dev_gui.get_available_content_region();
			m_asset_tree_state.reset();
			if (dev_gui.begin_child_window(m_asset_tree) == true)
			{
				Core::AssetLibrary& asset_library = m_editor.get_system<Core::ProjectManager>().get_asset_library();
				const Core::AssetNode& root_asset_node = asset_library.get_root();
				for (const Core::AssetNode* current_node : root_asset_node.get_children())
				{
					draw_asset_node(current_node, dev_gui);
				}
			}
			const bool window_clicked = dev_gui.is_window_hovered() && dev_gui.is_mouse_clicked(VadonApp::Platform::MouseButton::LEFT);
			dev_gui.end_child_window();

			if (m_asset_tree_state.clicked_node != nullptr)
			{
				m_selected_asset = m_asset_tree_state.clicked_node;
				if (m_asset_tree_state.double_clicked == true)
				{
					open_asset(m_selected_asset);
				}
			}
			else if (window_clicked == true)
			{
				m_selected_asset = nullptr;
			}
		}
		dev_gui.end_window();
	}

	void AssetBrowser::draw_asset_node(const Core::AssetNode* node, VadonApp::UI::Developer::GUISystem& dev_gui)
	{
		constexpr VadonApp::UI::Developer::GUISystem::TreeNodeFlags node_base_flags = VadonApp::UI::Developer::GUISystem::TreeNodeFlags::OPEN_ON_ARROW | VadonApp::UI::Developer::GUISystem::TreeNodeFlags::OPEN_ON_DOUBLE_CLICK;

		VadonApp::UI::Developer::GUISystem::TreeNodeFlags current_node_flags = node_base_flags;
		if (node == m_selected_asset)
		{
			current_node_flags |= VadonApp::UI::Developer::GUISystem::TreeNodeFlags::SELECTED;
		}
		if (node->get_children().empty() == true)
		{
			current_node_flags |= VadonApp::UI::Developer::GUISystem::TreeNodeFlags::LEAF;
		}

		get_asset_node_label(*node, m_asset_tree_state.node_label);
		const bool node_open = dev_gui.push_tree_node(node, m_asset_tree_state.node_label, current_node_flags);

		if ((dev_gui.is_item_clicked(VadonApp::Platform::MouseButton::LEFT) || dev_gui.is_item_clicked(VadonApp::Platform::MouseButton::RIGHT)) && (dev_gui.is_item_toggled_open() == false))
		{
			m_asset_tree_state.clicked_node = node;
			m_asset_tree_state.double_clicked = dev_gui.is_mouse_double_clicked(VadonApp::Platform::MouseButton::LEFT);
		}
		if (dev_gui.begin_popup_context_item() == true)
		{
			for (size_t current_item_index = 0; current_item_index < m_create_asset_context_menus.size(); ++current_item_index)
			{
				const UI::Developer::MenuItem& current_menu_item = m_create_asset_context_menus[current_item_index];
				if (dev_gui.add_menu_item(current_menu_item) == true)
				{
					switch (Vadon::Utilities::to_enum<CreateMenuOption>(static_cast<int32_t>(current_item_index)))
					{
					case CreateMenuOption::RESOURCE:
						m_create_resource_dialog.open();
						break;
					case CreateMenuOption::SCENE:
						// TODO!!!
						break;
					}
				}
			}
			dev_gui.end_popup();
		}

		if (node_open == true)
		{
			for (Core::AssetNode* child_node : node->get_children())
			{
				draw_asset_node(child_node, dev_gui);
			}
			dev_gui.pop_tree_node();
		}
	}

	void AssetBrowser::create_resource()
	{
		m_pending_asset_type = Core::AssetType::RESOURCE;
		if (m_selected_asset != nullptr)
		{
			Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
			const Vadon::Core::RootDirectoryHandle project_root_dir = project_manager.get_active_project().root_dir_handle;

			Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
			const std::string path = file_system.get_absolute_path(Vadon::Core::FileSystemPath{ .root_directory = project_root_dir, .path = m_selected_asset->get_path() });
			m_save_file_dialog.open_at(path);
			m_save_file_dialog.clear_file_name_input();
		}
		m_save_file_dialog.open();
	}

	void AssetBrowser::save_asset()
	{
		const Core::AssetType pending_asset_type = m_pending_asset_type;
		m_pending_asset_type = Core::AssetType::NONE;

		Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
		const Vadon::Core::RootDirectoryHandle project_root_dir = project_manager.get_active_project().root_dir_handle;

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		const std::string relative_path = file_system.get_relative_path(m_save_file_dialog.get_entered_file_path(), project_root_dir);

		switch (pending_asset_type)
		{
		case Core::AssetType::RESOURCE:
			if (save_resource(relative_path + Core::AssetLibrary::get_asset_type_file_extension(Core::AssetType::RESOURCE)) == false)
			{
				return;
			}
			break;
		default:
			return;
		}

		Core::AssetLibrary& asset_library = m_editor.get_system<Core::ProjectManager>().get_asset_library();
		Core::AssetNode* new_asset = asset_library.create_asset(pending_asset_type, relative_path);
		if (new_asset == nullptr)
		{
			// TODO: error!
			return;
		}

		m_selected_asset = new_asset;
	}

	bool AssetBrowser::save_resource(std::string_view path)
	{
		// TODO: check to make sure path is within project!		
		// First try to create the resource
		Model::ResourceSystem& editor_resource_system = m_editor.get_system<Model::ModelSystem>().get_resource_system();
		Model::Resource* new_resource = editor_resource_system.create_resource(m_create_resource_dialog.get_selected_resource_type());

		if (new_resource == nullptr)
		{
			m_editor.get_engine_core().log_error("Asset browser: failed to create resource!\n");
			return false;
		}

		Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
		const Vadon::Core::RootDirectoryHandle project_root_dir = project_manager.get_active_project().root_dir_handle;

		const Model::ResourcePath new_resource_path = { .root_directory = project_root_dir, .path = std::string(path) };

		new_resource->set_path(new_resource_path);
		if (new_resource->save() == false)
		{
			// TODO: delete resource?
			m_editor.get_engine_core().log_error("Asset browser: failed to save resource!\n");
			return false;
		}

		m_editor.get_engine_core().log_message(std::format("Saved new resource to \"{}\"\n", path));
		return true;
	}

	void AssetBrowser::open_asset(const Core::AssetNode* asset)
	{
		switch (asset->get_info().type)
		{
		case Core::AssetType::RESOURCE:
		{
			Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
			const Vadon::Core::RootDirectoryHandle project_root_dir = project_manager.get_active_project().root_dir_handle;

			Model::ResourceSystem& editor_resource_system = m_editor.get_system<Model::ModelSystem>().get_resource_system();

			// Attempt to import as resource
			// FIXME: what if it's not matching?
			const Model::ResourceID resource_id = editor_resource_system.get_database().import_resource(Model::ResourcePath{ .root_directory = project_root_dir, .path = asset->get_path() });
			if (resource_id.is_valid() == false)
			{
				// TODO: error!
				return;
			}

			Model::Resource* resource = editor_resource_system.get_resource(resource_id);
			if (resource != nullptr)
			{
				if (resource->load() == true)
				{
					// FIXME: move to View!
					m_editor.get_system<View::ViewSystem>().get_view_model().set_active_resource(resource);
				}
				else
				{
					// TODO: error!
				}
			}
			else
			{
				// TODO: error!
			}
		}
		break;
		}
	}
}