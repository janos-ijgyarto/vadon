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
		QString name;
		QString path;
		AssetType type = AssetType::NONE;
		QUuid file_id;

		bool operator==(const AssetInfo& other) const
		{
			return (path == other.path) && (type == other.type) && (file_id == other.file_id);
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
	};
}
#endif