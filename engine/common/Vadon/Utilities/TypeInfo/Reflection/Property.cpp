#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>

#include <Vadon/Utilities/TypeInfo/Registry.hpp>

namespace Vadon::Utilities
{
	PropertyCategory PropertyInfo::get_category() const
	{
		if (data_type == TypeRegistry::get_type_id(get_base_type_uuid(::Vadon::Foundation::BaseType::ARRAY)))
		{
			return PropertyCategory::ARRAY;
		}

		if (is_base_type(base_info.type))
		{
			return PropertyCategory::TRIVIAL;
		}

		return PropertyCategory::OBJECT;
	}
}