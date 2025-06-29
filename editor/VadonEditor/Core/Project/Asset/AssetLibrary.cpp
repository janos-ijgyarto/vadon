#include <VadonEditor/Core/Project/Asset/AssetLibrary.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

namespace
{
	// FIXME: make this more modular!
	constexpr const char* internal_get_asset_type_file_extension(VadonEditor::Core::AssetType type)
	{
		switch (type)
		{
		case VadonEditor::Core::AssetType::RESOURCE:
			return ".vdrc";
		case VadonEditor::Core::AssetType::SCENE:
			return ".vdsc";
		}

		return "";
	}

	VadonEditor::Core::AssetType get_asset_node_type(const std::filesystem::path& path)
	{
		const std::string extension = path.extension().string();
		if (extension == internal_get_asset_type_file_extension(VadonEditor::Core::AssetType::SCENE))
		{
			return VadonEditor::Core::AssetType::SCENE;
		}
		else if (extension == internal_get_asset_type_file_extension(VadonEditor::Core::AssetType::RESOURCE))
		{
			return VadonEditor::Core::AssetType::RESOURCE;
		}

		return VadonEditor::Core::AssetType::NONE;
	}
}

namespace VadonEditor::Core
{
	AssetNode* AssetLibrary::create_asset(AssetType type, std::string_view path)
	{
		if (path.empty() == true)
		{
			// TODO: error!
			return nullptr;
		}

		AssetNode* parent_node = find_node(path);
		if (parent_node == nullptr)
		{
			// TODO: error!
			return nullptr;
		}

		const std::string new_asset_path = std::string(path) + get_asset_type_file_extension(type);
		return create_asset_node(type, std::filesystem::path(new_asset_path).filename().string(), parent_node);
	}

	const AssetNode* AssetLibrary::find_node(std::string_view path) const
	{
		if (path.empty() == true)
		{
			return nullptr;
		}

		const AssetNode* candidate_node = &m_root;

		std::filesystem::path fs_path = path;
		for (const auto& current_element : fs_path)
		{
			const std::string current_element_string = current_element.string();
			bool candidate_found = false;
			for (const AssetNode* candidate_child : candidate_node->get_children())
			{
				if (candidate_child->get_info().name == current_element_string)
				{
					candidate_node = candidate_child;
					candidate_found = true;
					break;
				}
			}

			if (candidate_found == false)
			{
				return candidate_node;
			}
		}

		return candidate_node;
	}

	void AssetLibrary::rebuild_asset_tree()
	{
		m_root.clear();

		ProjectManager& project_manager = m_editor.get_system<ProjectManager>();
		const Vadon::Core::RootDirectoryHandle project_root_dir = project_manager.get_active_project().root_dir_handle;

		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		std::filesystem::path root_path = file_system.get_absolute_path(Vadon::Core::FileSystemPath{ .root_directory = project_root_dir });

		build_asset_tree_recursive(m_root, root_path);
	}

	const char* AssetLibrary::get_asset_type_file_extension(AssetType type)
	{
		return internal_get_asset_type_file_extension(type);
	}

	AssetLibrary::AssetLibrary(Editor& editor)
		: m_editor(editor)
	{}

	void AssetLibrary::build_asset_tree_recursive(AssetNode& parent, const std::filesystem::path& path)
	{
		for (const auto& directory_entry : std::filesystem::directory_iterator(path))
		{
			if (directory_entry.is_regular_file() == true)
			{
				const AssetType asset_type = get_asset_node_type(directory_entry.path());
				if (asset_type == AssetType::NONE)
				{
					continue;
				}

				create_asset_node(asset_type, directory_entry.path().filename().string(), &parent);
			}
			else if (directory_entry.is_directory() == true)
			{
				AssetNode* directory_node = create_asset_node(AssetType::NONE, directory_entry.path().stem().string(), &parent);
				build_asset_tree_recursive(*directory_node, directory_entry.path());
			}
		}
	}

	AssetNode* AssetLibrary::create_asset_node(AssetType type, std::string_view name, AssetNode* parent)
	{
		AssetNode* new_node = new AssetNode();
		new_node->m_parent = parent;

		AssetInfo& info = new_node->m_info;
		info.name = name;
		info.type = type;

		parent->m_children.push_back(new_node);

		return new_node;
	}
}