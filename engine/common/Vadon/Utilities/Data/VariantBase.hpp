#ifndef VADON_UTILITIES_DATA_VARIANTBASE_HPP
#define VADON_UTILITIES_DATA_VARIANTBASE_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
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
		Vadon::Scene::ResourceHandle, Utilities::UUID,
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

	template<Vadon::Scene::is_resource_id T>
	Variant to_variant(const T& value)
	{
		return to_variant<Vadon::Scene::ResourceID>(value);
	}

	template<Vadon::Scene::is_resource_handle T>
	Variant to_variant(const T& value)
	{
		return to_variant<Vadon::Scene::ResourceHandle>(value);
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

	template<Vadon::Scene::is_resource_id T>
	T from_variant(const Variant& variant)
	{
		return T(from_variant<Vadon::Scene::ResourceID>(variant));
	}

	template<Vadon::Scene::is_resource_handle T>
	T from_variant(const Variant& variant)
	{
		return T(from_variant<Vadon::Scene::ResourceHandle>(variant));
	}
}
#endif