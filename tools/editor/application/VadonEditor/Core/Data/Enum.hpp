#ifndef VADONEDITOR_CORE_DATA_ENUM_HPP
#define VADONEDITOR_CORE_DATA_ENUM_HPP
#include <Vadon/Foundation/TypeInfo/Metadata.hpp>
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
#include <QHash>
#include <QUuid>
namespace VadonEditor::Core
{
	struct EnumValueData
	{
		QUuid id;
		QHash<QString, QByteArray> metadata;

		QString find_metadata(const char* key) const;
		QString find_metadata(::Vadon::Foundation::CommonPropertyMetadata::Key key) const { return find_metadata(::Vadon::Foundation::CommonPropertyMetadata::key_string(key)); }

		QString get_name() const;
	};

	struct EnumData
	{
		QUuid id;
		QHash<QString, QByteArray> metadata;
		QHash<QUuid, EnumValueData> values;

		const EnumValueData* find_value_data(const QUuid& value_uuid) const;

		QString find_metadata(const char* key) const;
		QString find_metadata(::Vadon::Foundation::CommonTypeMetadata::Key key) const { return find_metadata(::Vadon::Foundation::CommonTypeMetadata::key_string(key)); }

		QString get_name() const;
	};
}
#endif