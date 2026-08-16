#ifndef VADONEDITOR_UTILITIES_UUID_HPP
#define VADONEDITOR_UTILITIES_UUID_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
#include <QRegularExpression>
#include <QUuid>
namespace VadonEditor::Utilities
{
	// FIXME: implement constexpr conversions from Vadon UUID strings to Qt UUIDs

	inline bool is_uuid_valid(const QUuid& uuid)
	{
		return uuid.isNull() == false;
	}

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

	inline QString serialize_labeled_uuid(QStringView label, const QUuid& uuid)
	{
		const QString sanitized_label = QString(label).toLower().replace(QRegularExpression("[^a-zA-Z0-9]"), "_");
		return QString("%1|%2").arg(sanitized_label).arg(uuid_to_base64_string(uuid));
	}

	inline QString create_uuid_key_string(const QUuid& uuid, QStringView label, bool labeled)
	{
		if ((labeled == true) && (label.isEmpty() == false))
		{
			return VadonEditor::Utilities::serialize_labeled_uuid(label, uuid);
		}
		else
		{
			return VadonEditor::Utilities::uuid_to_base64_string(uuid);
		}
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

	inline QString uuid_to_hex_string(const QUuid& uuid)
	{
		QString uuid_string;
		uuid_string.resize(::Vadon::Foundation::UUID::c_uuid_width * 2);

		constexpr auto half_byte_to_hex = +[](const char c) {
			if (c < 10)
			{
				return char('0' + c);
			}
			else
			{
				return char('A' + (c - 10));
			}
			};

		constexpr char half_byte_mask = ((1 << 4) - 1);
		size_t char_index = 0;
		const QUuid::Id128Bytes uuid_bytes = uuid.toBytes();
		for (size_t index = 0; index < ::Vadon::Foundation::UUID::c_uuid_width; ++index)
		{
			const char uuid_byte = uuid_bytes.data[index];

			uuid_string[char_index] = half_byte_to_hex((uuid_byte >> 4) & half_byte_mask);
			uuid_string[char_index + 1] = half_byte_to_hex(uuid_byte & half_byte_mask);
			char_index += 2;
		}

		return uuid_string;
	}
}
#endif