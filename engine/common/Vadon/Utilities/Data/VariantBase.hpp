#ifndef VADON_UTILITIES_DATA_VARIANTBASE_HPP
#define VADON_UTILITIES_DATA_VARIANTBASE_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
#include <Vadon/Math/Color.hpp>
#include <Vadon/Math/Vector.hpp>
#include <Vadon/Utilities/Container/Box.hpp>
#include <Vadon/Utilities/TypeInfo/TypeList/VariantTypeList.hpp>
namespace Vadon::Utilities
{
	// Used by function bindings to indicate a void function
	enum class NoReturnValue : unsigned char {};

	struct VariantArray;
	struct VariantDictionary;

	using BoxedVariantArray = Utilities::Box<VariantArray>;
	using BoxedVariantDictionary = Utilities::Box<VariantDictionary>;

	using Variant = std::variant<std::monostate, int, uint32_t, float, bool, std::string,
		Math::Vector2, Math::Vector2i, Math::Vector3, Math::Vector3i, Math::Vector4,
		Math::ColorRGBA,
		::Vadon::Foundation::UUID, // TODO: allow handles?
		BoxedVariantArray, BoxedVariantDictionary, NoReturnValue>;

	template<typename T>
	concept is_trivial_variant_type = type_list_has_type_v<T, Variant>;

	template<typename T>
	Variant to_variant(const T& value)
	{
		static_assert(false, "Type not supported!");
		return Variant();
	}

	template<is_trivial_variant_type T>
	Variant to_variant(const T& value)
	{
		return Variant(value);
	}

	template<typename T>
	T from_variant(const Variant& variant)
	{
		static_assert(false, "Type not supported!");
		return T();
	}
	
	template<is_trivial_variant_type T>
	T from_variant(const Variant& variant)
	{
		return std::get<T>(variant);
	}

	// FIXME: have to "forward declare" this for Resource IDs because of how arrays are declared
	// Need to revise to make sure we have an overload for arrays that contain ResourceIDs
	template<typename T>
	concept is_derived_uuid = std::is_base_of_v<::Vadon::Foundation::UUID, T> && (std::is_same_v<::Vadon::Foundation::UUID, T> == false);

	template<is_derived_uuid T>
	Variant to_variant(const T& value)
	{
		return to_variant<::Vadon::Foundation::UUID>(value);
	}

	template<is_derived_uuid T>
	T from_variant(const Variant& variant)
	{
		return T(from_variant<::Vadon::Foundation::UUID>(variant));
	}
}
#endif