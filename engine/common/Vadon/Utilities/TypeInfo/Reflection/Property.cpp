#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>

#include <Vadon/Utilities/Debugging/Assert.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>

#include <Vadon/Foundation/Model/Resource/Resource.hpp>
#include <Vadon/Foundation/TypeInfo/Object.hpp>

namespace Vadon::Utilities
{
	::Vadon::Foundation::Property::Category PropertyInfo::get_category(const ::Vadon::Foundation::UUID& type_uuid)
	{
		if (type_uuid.is_valid() == false)
		{
			return ::Vadon::Foundation::Property::Category::INVALID;
		}

		if (type_uuid == get_base_type_uuid(::Vadon::Foundation::BaseType::ARRAY))
		{
			return ::Vadon::Foundation::Property::Category::ARRAY;
		}

		if (type_uuid == get_base_type_uuid(::Vadon::Foundation::BaseType::DICTIONARY))
		{
			return ::Vadon::Foundation::Property::Category::DICTIONARY;
		}

		if (type_uuid == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::ResourceIDSchema::c_type_uuid))
		{
			return ::Vadon::Foundation::Property::Category::RESOURCE;
		}

		if (is_base_type(type_uuid))
		{
			return ::Vadon::Foundation::Property::Category::TRIVIAL;
		}

		if (type_uuid == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			return ::Vadon::Foundation::Property::Category::OBJECT;
		}

		// Fallback: assume it's an object
		return ::Vadon::Foundation::Property::Category::OBJECT;
	}
}