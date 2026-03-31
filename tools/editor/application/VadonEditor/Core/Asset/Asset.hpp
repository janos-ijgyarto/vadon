#ifndef VADONEDITOR_CORE_ASSET_ASSET_HPP
#define VADONEDITOR_CORE_ASSET_ASSET_HPP
#include <QUuid>
namespace VadonEditor::Core
{
	enum class AssetType
	{
		RESOURCE,
		SCENE,
		IMPORTED_FILE,
		FOLDER,
		TYPE_COUNT,
		NONE = TYPE_COUNT
	};

	struct AssetInfo
	{
		static constexpr int c_invalid_file_id = 0;

		int id = c_invalid_file_id;
		QString path; // NOTE: all assets use *RELATIVE* paths w.r.t the project root
		AssetType type = AssetType::NONE;

		bool is_valid() const { return type != AssetType::NONE; }

		bool operator==(const AssetInfo& other) const
		{
			return (path == other.path) && (type == other.type) && (id == other.id);
		}

		// FIXME: make this more modular!
		static constexpr const char* get_file_suffix(AssetType type)
		{
			switch (type)
			{
			case AssetType::RESOURCE:
				return "vdrc";
			case AssetType::SCENE:
				return "vdsc";
			case AssetType::IMPORTED_FILE:
				return "vdimport";
			}

			return "";
		}

		// NOTE: assumes path will not end in a file suffix, and can simply attach the suffix
		static QString get_file_path(const QString& path, AssetType type)
		{
			return path + "." + Core::AssetInfo::get_file_suffix(type);
		}
	};
}
#endif