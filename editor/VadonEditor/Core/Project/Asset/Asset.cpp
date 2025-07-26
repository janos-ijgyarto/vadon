#include <VadonEditor/Core/Project/Asset/Asset.hpp>

#include <filesystem>

namespace VadonEditor::Core
{
	std::string AssetNode::get_path() const
	{
		const AssetInfo& info = get_info();
		std::filesystem::path path = info.name;
		AssetNode* parent = m_parent;
		while (parent != nullptr)
		{
			path = parent->get_info().name / path;
			parent = parent->m_parent;
		}

		if (info.type != AssetType::NONE)
		{
			path.replace_extension(AssetInfo::get_file_extension(info.type));
		}

		return path.string();
	}

	AssetNode::AssetNode()
		: m_parent(nullptr)
	{

	}

	void AssetNode::clear()
	{
		for (AssetNode* current_child : m_children)
		{
			delete current_child;
		}
		m_children.clear();
	}
}