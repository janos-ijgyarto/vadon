#include <VadonEditor/Core/Data/Property.hpp>

#include <VadonEditor/Core/Data/Type.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

namespace VadonEditor::Core
{
	QString PropertyData::find_metadata(const char* key) const
	{
		auto metadata_it = metadata.find(key);
		if (metadata_it == metadata.end())
		{
			return QString();
		}

		return metadata_it->constData();
	}

	PropertyCategory PropertyData::get_category() const
	{
		const ::Vadon::Foundation::BaseType base_type = TypeData::get_base_type(Utilities::vadon_uuid_to_qt_uuid(info.type));
		switch (base_type)
		{
		case ::Vadon::Foundation::BaseType::UUID:
		{
			// Resource type only if it has valid metadata
			if (find_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE).isEmpty() == false)
			{
				return PropertyCategory::RESOURCE;
			}
		}
		break;
		case ::Vadon::Foundation::BaseType::ARRAY:
			return PropertyCategory::ARRAY;
		}

		// Fall back on trivial
		return PropertyCategory::TRIVIAL;
	}

	QString PropertyData::get_name() const
	{
		QString name = find_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME);
		if (name.isEmpty() == true)
		{
			name = QString("Property_%1").arg(Utilities::vadon_uuid_to_qt_uuid(info.id).toString(QUuid::StringFormat::WithoutBraces));
		}

		return name;
	}

	QUuid PropertyData::get_data_type() const
	{
		switch (get_category())
		{
		case PropertyCategory::RESOURCE:
		{
			// FIXME: we should also make sure the resource type is registered!
			const QString uuid_string = find_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE);
			const ::Vadon::Foundation::UUID resource_type_uuid = Utilities::qt_uuid_to_vadon_uuid(Utilities::base64_string_to_uuid(uuid_string));
			Q_ASSERT_X(resource_type_uuid.is_valid() == true, "VadonEditor::Core::PropertyData", "Invalid resource type!");
			return Utilities::vadon_uuid_to_qt_uuid(resource_type_uuid);
		}
		case PropertyCategory::ARRAY:
		{
			const QString uuid_string = find_metadata(::Vadon::Foundation::CommonPropertyMetadata::ARRAY_TYPE);
			Q_ASSERT_X(uuid_string.isEmpty() == false, "VadonEditor::Core::PropertyData", "Invalid array type!");

			const ::Vadon::Foundation::UUID array_type_uuid = Utilities::qt_uuid_to_vadon_uuid(Utilities::base64_string_to_uuid(uuid_string));
			Q_ASSERT_X(array_type_uuid.is_valid() == true, "VadonEditor::Core::PropertyData", "Invalid array type!");
			return Utilities::vadon_uuid_to_qt_uuid(array_type_uuid);
		}
		}

		// Fallback is the actual data type
		return Utilities::vadon_uuid_to_qt_uuid(info.type);
	}
}