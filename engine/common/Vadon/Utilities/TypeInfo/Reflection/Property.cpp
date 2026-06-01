#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>

#include <Vadon/Utilities/Debugging/Assert.hpp>

#include <Vadon/Utilities/TypeInfo/Registry.hpp>

namespace Vadon::Utilities
{
	PropertyCategory PropertyInfo::get_category() const
	{
		if (data_type == TypeRegistry::get_type_id(get_base_type_uuid(::Vadon::Foundation::BaseType::ARRAY)))
		{
			const ::Vadon::Foundation::BaseType base_type = base_type_from_uuid(base_info.type);
			switch (base_type)
			{
			case ::Vadon::Foundation::BaseType::ARRAY:
			case ::Vadon::Foundation::BaseType::DICTIONARY:
				VADON_ERROR("Nested containers are not allowed!");
				return PropertyCategory::INVALID;
			case ::Vadon::Foundation::BaseType::INVALID:
				// Assume it's an array of objects
				return PropertyCategory::TYPED_OBJECT_ARRAY;
			default:
			{
				if (base_info.type == VADON_GET_TYPE_UUID(ObjectPointer))
				{
					return PropertyCategory::GENERIC_OBJECT_ARRAY;
				}
				else
				{
					return PropertyCategory::TRIVIAL_ARRAY;
				}
			}
			}
		}

		if (data_type == TypeRegistry::get_type_id(VADON_GET_TYPE_UUID(ObjectPointer)))
		{
			return PropertyCategory::GENERIC_OBJECT;
		}

		if (is_base_type(base_info.type))
		{
			return PropertyCategory::TRIVIAL;
		}

		return PropertyCategory::TYPED_OBJECT;
	}
}