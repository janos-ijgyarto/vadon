#ifndef VADONEDITOR_CORE_DATA_PROPERTY_HPP
#define VADONEDITOR_CORE_DATA_PROPERTY_HPP
#include <Vadon/Foundation/TypeInfo/Metadata.hpp>
#include <Vadon/Foundation/TypeInfo/Property.hpp>
#include <QHash>
#include <QUuid>
namespace VadonEditor::Core
{
	enum class PropertyCategory
	{
		TRIVIAL,
		RESOURCE,
		ARRAY,
		OBJECT
	};

	struct PropertyData
	{
		::Vadon::Foundation::Property info;
		QHash<QString, QByteArray> metadata;

		QString find_metadata(const char* key) const;
		QString find_metadata(::Vadon::Foundation::CommonPropertyMetadata::Key key) const { return find_metadata(::Vadon::Foundation::CommonPropertyMetadata::key_string(key)); }

		PropertyCategory get_category() const;

		QString get_name() const;
		QUuid get_data_type() const;
	};
}
#endif