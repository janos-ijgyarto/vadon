#include <VadonEditor/Core/Project/Asset/AssetLibrary.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <filesystem>

namespace VadonEditor::Core
{
	AssetNode* AssetLibrary::create_node(const AssetInfo& info, std::string_view path)
	{
		if (path.empty() == true)
		{
			// TODO: error!
			return nullptr;
		}

		const std::filesystem::path parent_path = std::filesystem::path(path).parent_path();

		AssetNode* parent_node = find_node(parent_path.generic_string());
		if (parent_node == nullptr)
		{
			// Parent not found, so we have to create the path
			parent_node = &m_root;

			for (const std::filesystem::path& current_element : parent_path)
			{
				const std::string current_element_string = current_element.generic_string();
				AssetNode* next_parent = nullptr;
				for (AssetNode* candidate_child : parent_node->get_children())
				{
					if (candidate_child->get_info().name == current_element_string)
					{
						next_parent = candidate_child;
						break;
					}
				}

				if (next_parent == nullptr)
				{
					AssetInfo node_info;
					node_info.type = AssetType::NONE;
					node_info.name = current_element_string;
					next_parent = internal_create_node(parent_node, node_info);
				}

				parent_node = next_parent;
			}
		}

		return internal_create_node(parent_node, info);
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
			const std::string current_element_string = current_element.generic_string();
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
				return nullptr;
			}
		}

		return candidate_node;
	}

	AssetLibrary::AssetLibrary(Editor& editor)
		: m_editor(editor)
	{}

	bool AssetLibrary::initialize()
	{
		// TODO: anything?
		return true;
	}

	AssetNode* AssetLibrary::internal_create_node(AssetNode* parent, const AssetInfo& info)
	{
		for (const AssetNode* current_child : parent->m_children)
		{
			if (current_child->get_info() == info)
			{
				// TODO: error!
				return nullptr;
			}
		}

		AssetNode* new_node = new AssetNode();
		new_node->m_parent = parent;
		new_node->m_info = info;

		parent->m_children.push_back(new_node);

		return new_node;
	}
}