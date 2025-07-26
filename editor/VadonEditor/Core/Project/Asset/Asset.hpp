#ifndef VADONEDITOR_CORE_PROJECT_ASSET_ASSET_HPP
#define VADONEDITOR_CORE_PROJECT_ASSET_ASSET_HPP
#include <Vadon/Core/File/File.hpp>
#include <vector>
namespace VadonEditor::Core
{
	enum class AssetType
	{
		RESOURCE,
		SCENE,
		IMPORTED_FILE,
		TYPE_COUNT,
		NONE = TYPE_COUNT
		// TODO: anything else?
	};

	struct AssetInfo
	{
		std::string name;
		AssetType type = AssetType::NONE;
		Vadon::Core::FileID file_id;

		bool operator==(const AssetInfo& other) const
		{
			return (name == other.name) && (type == other.type) && (file_id == other.file_id);
		}

		// FIXME: make this more modular!
		static constexpr const char* get_file_extension(AssetType type)
		{
			switch (type)
			{
			case AssetType::RESOURCE:
				return ".vdrc";
			case AssetType::SCENE:
				return ".vdsc";
			case AssetType::IMPORTED_FILE:
				return ".vdimport";
			}

			return "";
		}
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