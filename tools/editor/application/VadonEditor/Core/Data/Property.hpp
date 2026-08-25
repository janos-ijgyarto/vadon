#ifndef VADONEDITOR_CORE_DATA_PROPERTY_HPP
#define VADONEDITOR_CORE_DATA_PROPERTY_HPP
#include <Vadon/Foundation/TypeInfo/Metadata.hpp>
#include <Vadon/Foundation/TypeInfo/Property.hpp>
#include <QHash>
#include <QUuid>
namespace VadonEditor::Core
{
	struct PropertyData
	{
		::Vadon::Foundation::Property info;
		QList<QUuid> type_list;
		QHash<QString, QByteArray> metadata;
		::Vadon::Foundation::CommonPropertyMetadata::Flags flags = ::Vadon::Foundation::CommonPropertyMetadata::Flags::NONE;

		QString find_metadata(const char* key) const;
		QString find_metadata(::Vadon::Foundation::CommonPropertyMetadata::Key key) const { return find_metadata(::Vadon::Foundation::CommonPropertyMetadata::key_string(key)); }

		QUuid get_root_type() const { return type_list.front(); }

		static ::Vadon::Foundation::Property::Category get_category(const QUuid& root_type_id);

		// NOTE: convenience function to check metadata
		QString get_name() const;
	};
}
#endif