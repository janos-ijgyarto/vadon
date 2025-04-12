#ifndef VADON_UTILITIES_TYPEINFO_REFLECTION_ERASEDPROPERTY_HPP
#define VADON_UTILITIES_TYPEINFO_REFLECTION_ERASEDPROPERTY_HPP
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>
#include <Vadon/Utilities/TypeInfo/TypeErasure.hpp>
namespace Vadon::Utilities
{
	template<typename T>
	Property create_type_erased_property(std::string_view name, const T& value)
	{
		return Property{
			.name = std::string(name),
			.data_type = Vadon::Utilities::get_erased_data_type_id<T>(),
			.value = value
		};
	}
}
#endif