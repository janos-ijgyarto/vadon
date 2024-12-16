#ifndef VADONEDITOR_CORE_PROJECT_ASSET_ASSET_HPP
#define VADONEDITOR_CORE_PROJECT_ASSET_ASSET_HPP
#include <vector>
namespace VadonEditor::Core
{
	enum class AssetType
	{
		NONE,
		RESOURCE,
		SCENE
		// TODO: anything else?
	};

	struct AssetInfo
	{
		std::string name;
		AssetType type = AssetType::NONE;
	};

	// FIXME: need generalized "TreeNode" implementation to use in tree views!
	class AssetNode
	{
	public:
		~AssetNode() { clear(); }
		const AssetInfo& get_info() const { return m_info; }

		std::string get_path() const;

		const std::vector<AssetNode*>& get_children() const { return m_children; }
	private:
		AssetNode();

		void clear();

		AssetInfo m_info;

		AssetNode* m_parent;
		std::vector<AssetNode*> m_children;

		friend class AssetLibrary;
	};
}
#endif