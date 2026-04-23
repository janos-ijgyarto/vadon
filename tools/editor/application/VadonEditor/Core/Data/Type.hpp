#ifndef VADONEDITOR_CORE_DATA_TYPE_HPP
#define VADONEDITOR_CORE_DATA_TYPE_HPP
#include <VadonEditor/Core/Data/Property.hpp>
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
#include <QUuid>
namespace VadonEditor::Core
{
	struct TypeData
	{
		QHash<QString, QByteArray> metadata;
		QHash<QUuid, PropertyData> properties;
		QList<::Vadon::Foundation::UUID> property_list;
		::Vadon::Foundation::TypeInfo info;

		const PropertyData* find_property_data(const QUuid& property_uuid) const;

		QString find_metadata(const char* key) const;
		QString find_metadata(::Vadon::Foundation::CommonTypeMetadata::Key key) const { return find_metadata(::Vadon::Foundation::CommonTypeMetadata::key_string(key)); }

		QString get_name() const;

		static QUuid get_base_type_uuid(::Vadon::Foundation::BaseType type);
		static ::Vadon::Foundation::BaseType get_base_type(const QUuid& type_uuid);

		static QVariant serialize_base_type(::Vadon::Foundation::BaseType type, const QVariant& value);
		static QVariant deserialize_base_type(::Vadon::Foundation::BaseType type, const QVariant& value);
	};
}
#endif