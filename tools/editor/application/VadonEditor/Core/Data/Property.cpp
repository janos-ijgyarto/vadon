#include <VadonEditor/Core/Data/Property.hpp>

#include <VadonEditor/Core/Data/Object.hpp>
#include <VadonEditor/Core/Data/Type.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/TypeInfo/Object.hpp>

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
		const QUuid type_uuid = Utilities::vadon_uuid_to_qt_uuid(info.type);
		const QUuid generic_obj_uuid = DataObject::get_object_type_uuid();
		if (type_uuid == generic_obj_uuid)
		{
			return PropertyCategory::GENERIC_OBJECT;
		}

		const ::Vadon::Foundation::BaseType base_type = TypeData::get_base_type(type_uuid);
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
		case ::Vadon::Foundation::BaseType::INVALID:
		{
			// We need metadata to know what kind of object this is
			QString metadata_string = find_metadata(::Vadon::Foundation::CommonPropertyMetadata::OBJECT_TYPE);
			if (metadata_string.isEmpty() == false)
			{
				return PropertyCategory::TYPED_OBJECT;
			}

			metadata_string = find_metadata(::Vadon::Foundation::CommonPropertyMetadata::ARRAY_TYPE);			
			if (metadata_string.isEmpty() == false)
			{
				// Find the underlying type of the array
				const QUuid array_type = Utilities::base64_string_to_uuid(metadata_string);
				if (array_type == generic_obj_uuid)
				{
					return PropertyCategory::GENERIC_OBJECT_ARRAY;
				}

				const ::Vadon::Foundation::BaseType array_type_base_type = TypeData::get_base_type(array_type);
				switch (array_type_base_type)
				{
				case ::Vadon::Foundation::BaseType::INVALID:
					// Assume it's an array of typed objects
					return PropertyCategory::TYPED_OBJECT_ARRAY;
				default:
					return PropertyCategory::TRIVIAL_ARRAY;
				}
			}

			Q_UNREACHABLE();
		}
		break;
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
			const QUuid resource_type_uuid = Utilities::base64_string_to_uuid(uuid_string);
			Q_ASSERT_X(resource_type_uuid.isNull() == false, "VadonEditor::Core::PropertyData", "Invalid resource type!");
			return resource_type_uuid;
		}
		case PropertyCategory::TRIVIAL_ARRAY:
		case PropertyCategory::GENERIC_OBJECT_ARRAY:
		case PropertyCategory::TYPED_OBJECT_ARRAY:
		{
			const QString uuid_string = find_metadata(::Vadon::Foundation::CommonPropertyMetadata::ARRAY_TYPE);
			Q_ASSERT_X(uuid_string.isEmpty() == false, "VadonEditor::Core::PropertyData", "Invalid array type!");

			const QUuid array_type_uuid = Utilities::base64_string_to_uuid(uuid_string);
			Q_ASSERT_X(array_type_uuid.isNull() == false, "VadonEditor::Core::PropertyData", "Invalid array type!");
			return array_type_uuid;
		}
		case PropertyCategory::GENERIC_OBJECT:
			return Core::DataObject::get_object_type_uuid();
		case PropertyCategory::TYPED_OBJECT:
		{
			const QString uuid_string = find_metadata(::Vadon::Foundation::CommonPropertyMetadata::OBJECT_TYPE);
			Q_ASSERT_X(uuid_string.isEmpty() == false, "VadonEditor::Core::PropertyData", "Invalid object type!");

			const QUuid object_type_uuid = Utilities::base64_string_to_uuid(uuid_string);
			Q_ASSERT_X(object_type_uuid.isNull() == false, "VadonEditor::Core::PropertyData", "Invalid object type!");
			return object_type_uuid;
		}
		}

		// Fallback is the actual data type
		return Utilities::vadon_uuid_to_qt_uuid(info.type);
	}
}