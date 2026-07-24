#include <VadonEditor/Core/Data/Property.hpp>

#include <VadonEditor/Core/Data/Object.hpp>
#include <VadonEditor/Core/Data/Type.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Model/Resource/Resource.hpp>
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

	::Vadon::Foundation::Property::Category PropertyData::get_category(const QUuid& root_type_id)
	{
		// FIXME: duplicated by engine, could move it to a shared library?
		if (root_type_id.isNull() == true)
		{
			return ::Vadon::Foundation::Property::Category::INVALID;
		}

		if (root_type_id == TypeData::get_base_type_uuid(::Vadon::Foundation::BaseType::ARRAY))
		{
			return ::Vadon::Foundation::Property::Category::ARRAY;
		}

		if (root_type_id == TypeData::get_base_type_uuid(::Vadon::Foundation::BaseType::DICTIONARY))
		{
			return ::Vadon::Foundation::Property::Category::DICTIONARY;
		}

		if (root_type_id == Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ResourceIDSchema::c_type_uuid))
		{
			return ::Vadon::Foundation::Property::Category::RESOURCE;
		}

		if (TypeData::is_base_type(root_type_id))
		{
			return ::Vadon::Foundation::Property::Category::TRIVIAL;
		}

		if (root_type_id == Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			return ::Vadon::Foundation::Property::Category::OBJECT;
		}

		// Fallback: assume it's an object
		return ::Vadon::Foundation::Property::Category::OBJECT;
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
}