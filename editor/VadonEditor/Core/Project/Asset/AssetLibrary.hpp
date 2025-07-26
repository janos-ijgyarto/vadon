#ifndef VADONEDITOR_CORE_PROJECT_ASSET_ASSETLIBRARY_HPP
#define VADONEDITOR_CORE_PROJECT_ASSET_ASSETLIBRARY_HPP
#include <VadonEditor/Core/Project/Asset/Asset.hpp>
namespace VadonEditor::Core
{
	class Editor;

	class AssetLibrary
	{
	public:
		const AssetNode& get_root() { return m_root; }

		AssetNode* create_node(const AssetInfo& info, std::string_view path);

		const AssetNode* find_node(std::string_view path) const;
		AssetNode* find_node(std::string_view path) { return const_cast<AssetNode*>(std::as_const(*this).find_node(path)); }
	private:
		AssetLibrary(Editor& editor);

		bool initialize();

		static AssetNode* internal_create_node(AssetNode* parent, const AssetInfo& info);

		Editor& m_editor;
		AssetNode m_root;

		friend class ProjectManager;
	};
}
#endif