#ifndef VADON_UTILITIES_DATA_VARIANT_HPP
#define VADON_UTILITIES_DATA_VARIANT_HPP
#include <Vadon/Utilities/Data/VariantBase.hpp>
#include <Vadon/Utilities/Data/VariantArray.hpp>
#include <Vadon/Utilities/Data/VariantDictionary.hpp>

namespace Vadon::Utilities
{
	VADON_REGISTER_TYPE_UUID(NoReturnValue, "82a04f80-b040-4154-94a6-8a0212f5cff8");
	VADON_DECLARE_BASE_DATA_TYPE(NoReturnValue);

	VADON_REGISTER_TYPE_UUID(BoxedVariantArray, ::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::ARRAY).string);
	VADON_DECLARE_BASE_DATA_TYPE(BoxedVariantArray);

	VADON_REGISTER_TYPE_UUID(BoxedVariantDictionary, ::Vadon::Foundation::get_base_type_uuid_string(::Vadon::Foundation::BaseType::DICTIONARY).string);
	VADON_DECLARE_BASE_DATA_TYPE(BoxedVariantDictionary);
}
#endif