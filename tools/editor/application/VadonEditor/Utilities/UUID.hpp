#ifndef VADONEDITOR_UTILITIES_UUID_HPP
#define VADONEDITOR_UTILITIES_UUID_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
#include <QRegularExpression>
#include <QUuid>
namespace VadonEditor::Utilities
{
	// FIXME: implement constexpr conversions from Vadon UUID strings to Qt UUIDs

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

	inline QUuid vadon_uuid_string_to_qt_uuid(const ::Vadon::Foundation::UUIDString& uuid_string)
	{
		return QUuid::fromString(uuid_string.string);
	}

	inline QString serialize_labeled_uuid(const QString& label, const QUuid& uuid)
	{
		const QString sanitized_label = label.toLower().replace(QRegularExpression("[^a-zA-Z0-9]"), "_");
		return QString("%1|%2").arg(sanitized_label).arg(uuid_to_base64_string(uuid));
	}

	inline QUuid parse_labeled_uuid(const QString& uuid_string)
	{
		const qsizetype separator_index = uuid_string.indexOf('|');
		if (separator_index != -1)
		{
			return base64_string_to_uuid(uuid_string.sliced(separator_index + 1));
		}

		// Assume it's a regular UUID string
		return base64_string_to_uuid(uuid_string);
	}
}
#endif