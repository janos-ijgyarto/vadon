#ifndef VADONEDITOR_CORE_PROJECT_ASSET_ASSETLIBRARY_HPP
#define VADONEDITOR_CORE_PROJECT_ASSET_ASSETLIBRARY_HPP
#include <VadonEditor/Core/Project/Asset/Asset.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
#include <filesystem>
namespace VadonEditor::Core
{
	class Editor;

	class AssetLibrary
	{
	public:
		const AssetNode& get_root() { return m_root; }

		AssetNode* create_asset(AssetType type, std::string_view path);

		const AssetNode* find_node(std::string_view path) const;
		AssetNode* find_node(std::string_view path) { return const_cast<AssetNode*>(std::as_const(*this).find_node(path)); }

		static const char* get_asset_type_file_extension(AssetType type);
	private:
		AssetLibrary(Editor& editor);
		void rebuild_asset_tree();

		static void build_asset_tree_recursive(AssetNode& node, const std::filesystem::path& path);
		static AssetNode* create_asset_node(AssetType type, std::string_view name, AssetNode* parent);

		Editor& m_editor;
		AssetNode m_root;

		friend class ProjectManager;
	};
}
#endif