#include <VadonEditor/Core/Data/Type.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

namespace
{
	QUuid s_base_type_uuids[static_cast<::Vadon::Foundation::uint32>(::Vadon::Foundation::BaseType::TYPE_COUNT)] = {
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::INT32)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::UINT32)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::FLOAT)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::BOOL)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::STRING)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::VECTOR2)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::VECTOR2I)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::VECTOR3)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::VECTOR3I)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::VECTOR4)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::COLORRGBA)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::UUID)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::ARRAY)),
		VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::DICTIONARY))
	};
}

namespace VadonEditor::Core
{
	const PropertyData* TypeData::find_property_data(const QUuid& property_uuid) const
	{
		auto property_it = properties.find(property_uuid);

		if (property_it == properties.end())
		{
			// TODO: warning?
			return nullptr;
		}

		return &property_it.value();
	}

	QString TypeData::find_metadata(const char* key) const
	{
		auto metadata_it = metadata.find(key);
		if (metadata_it == metadata.end())
		{
			return QString();
		}

		return metadata_it->constData();
	}

	QString TypeData::get_name() const
	{
		QString name = find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
		if (name.isEmpty() == true)
		{
			name = QString("Type_%1").arg(Utilities::vadon_uuid_to_qt_uuid(info.id).toString(QUuid::StringFormat::WithoutBraces));
		}

		return name;
	}

	QUuid TypeData::get_base_type_uuid(::Vadon::Foundation::BaseType type)
	{
		return s_base_type_uuids[static_cast<size_t>(type)];
	}

	::Vadon::Foundation::BaseType TypeData::get_base_type(const QUuid& type_uuid)
	{
		if (type_uuid.isNull() == true)
		{
			return ::Vadon::Foundation::BaseType::INVALID;
		}

		for (int type_index = 0; type_index < static_cast<int>(::Vadon::Foundation::BaseType::TYPE_COUNT); ++type_index)
		{
			if (type_uuid == s_base_type_uuids[type_index])
			{
				return static_cast<::Vadon::Foundation::BaseType>(type_index);
			}
		}

		return ::Vadon::Foundation::BaseType::INVALID;
	}

	bool TypeData::is_base_type(const QUuid& type_uuid)
	{
		return get_base_type(type_uuid) != ::Vadon::Foundation::BaseType::INVALID;
	}

	QVariant TypeData::serialize_base_type(::Vadon::Foundation::BaseType type, const QVariant& value)
	{
		switch (type)
		{
		case ::Vadon::Foundation::BaseType::INVALID:
		{
			Q_ASSERT_X(false, "VadonEditor::Core::TypeData::serialize_base_type", "Invalid data type!");
			return QVariant();
		}
		case ::Vadon::Foundation::BaseType::UUID:
			// UUIDs are stored as base64 strings
			return VadonEditor::Utilities::uuid_to_base64_string(value.toUuid());
		default:
			return value;
		}
	}

	QVariant TypeData::deserialize_base_type(::Vadon::Foundation::BaseType type, const QVariant& value)
	{
		switch (type)
		{
		case ::Vadon::Foundation::BaseType::INVALID:
		{
			Q_ASSERT_X(false, "VadonEditor::Core::TypeData::deserialize_base_type", "Invalid data type!");
			return QVariant();
		}
		case ::Vadon::Foundation::BaseType::UUID:
			// UUIDs are stored as base64 strings
			return VadonEditor::Utilities::base64_string_to_uuid(value.toString());
		default:
			return value;
		}
	}
}