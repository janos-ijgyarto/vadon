#ifndef VADONEDITOR_UTILITIES_UUID_HPP
#define VADONEDITOR_UTILITIES_UUID_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
#include <QUuid>
namespace VadonEditor::Utilities
{
	inline QUuid vadon_uuid_to_qt_uuid(const ::Vadon::Foundation::UUID& uuid)
	{
		// NOTE: Qt defaults to Big Endian, worth taking a second look?
		return QUuid::fromBytes(uuid.data);
	}

	inline ::Vadon::Foundation::UUID qt_uuid_to_vadon_uuid(const QUuid& uuid)
	{
		// NOTE: Qt defaults to Big Endian, worth taking a second look?
		::Vadon::Foundation::UUID vadon_uuid;
		const QUuid::Id128Bytes uuid_bytes = uuid.toBytes();
		memcpy(vadon_uuid.data, &uuid_bytes, ::Vadon::Foundation::UUID::c_uuid_width);

		return vadon_uuid;
	}

	inline QUuid base64_string_to_uuid(const QString& string)
	{
		if (string.isEmpty())
		{
			return QUuid();
		}
		QByteArray uuid_arr = QByteArray::fromBase64(string.toUtf8());
		return QUuid::fromBytes(uuid_arr);
	}
		
	inline ::Vadon::Foundation::UUID base64_string_to_vadon_uuid(const QString& string)
	{
		return qt_uuid_to_vadon_uuid(base64_string_to_uuid(string));
	}

	inline QString uuid_to_base64_string(const QUuid& uuid)
	{
		const QUuid::Id128Bytes uuid_bytes = uuid.toBytes();
		return QString(QByteArray(QByteArrayView(uuid_bytes)).toBase64());
	}

	inline QString uuid_string_to_base64_string(const char* string)
	{
		const QUuid uuid = QUuid::fromString(string);
		return uuid_to_base64_string(uuid);
	}
}
#endif