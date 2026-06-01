#ifndef VADON_UTILITIES_DATA_VARIANTBASE_HPP
#define VADON_UTILITIES_DATA_VARIANTBASE_HPP
#include <Vadon/Math/Color.hpp>
#include <Vadon/Math/Vector.hpp>
#include <Vadon/Utilities/Container/Box.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
#include <Vadon/Utilities/TypeInfo/TypeList/VariantTypeList.hpp>
#include <string>
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
		BoxedVariantArray, BoxedVariantDictionary, ObjectPointer, NoReturnValue>;

	template<typename T>
	struct VariantTypeTrait
	{
		static Variant to_variant(const T& value)
		{
			static_assert(false, "Type not supported!");
			return Variant();
		}

		static T from_variant(const Variant& variant)
		{
			static_assert(false, "Type not supported!");
			return T();
		}
	};

	template<typename T>
	concept is_trivial_variant_type = std::is_same_v<T, std::monostate> 
		|| std::is_same_v<T, int>
		|| std::is_same_v<T, uint32_t>
		|| std::is_same_v<T, float>
		|| std::is_same_v<T, bool>
		|| std::is_same_v<T, std::string>
		|| std::is_same_v<T, int>
		|| std::is_same_v<T, ::Vadon::Math::Vector2>
		|| std::is_same_v<T, ::Vadon::Math::Vector2i>
		|| std::is_same_v<T, ::Vadon::Math::Vector3>
		|| std::is_same_v<T, ::Vadon::Math::Vector3i>
		|| std::is_same_v<T, ::Vadon::Math::Vector4>
		|| std::is_same_v<T, ::Vadon::Math::ColorRGBA>
		|| std::is_same_v<T, ::Vadon::Foundation::UUID>
		|| std::is_same_v<T, ::Vadon::Utilities::BoxedVariantArray>
		|| std::is_same_v<T, ::Vadon::Utilities::BoxedVariantDictionary>
		|| std::is_same_v<T, ::Vadon::Utilities::ObjectPointer>
		|| std::is_same_v<T, ::Vadon::Utilities::NoReturnValue>;

	template<is_trivial_variant_type T>
	struct VariantTypeTrait<T>
	{
		static Variant to_variant(const T& value)
		{
			return Variant(value);
		}

		static T from_variant(const Variant& variant)
		{
			return std::get<T>(variant);
		}
	};

	// FIXME: have to "forward declare" this for Resource IDs because of how arrays are declared
	// Need to revise to make sure we have an overload for arrays that contain ResourceIDs
	template<typename T>
	concept is_derived_uuid = std::is_base_of_v<::Vadon::Foundation::UUID, T> && (std::is_same_v<::Vadon::Foundation::UUID, T> == false);

	template<is_derived_uuid T>
	struct VariantTypeTrait<T>
	{
		static Variant to_variant(const T& value)
		{
			return VariantTypeTrait<::Vadon::Foundation::UUID>::to_variant(value);
		}

		static T from_variant(const Variant& variant)
		{
			return T(VariantTypeTrait<::Vadon::Foundation::UUID>::from_variant(variant));
		}
	};
}
#endif