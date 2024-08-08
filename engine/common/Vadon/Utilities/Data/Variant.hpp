#ifndef VADON_UTILITIES_DATA_VARIANT_HPP
#define VADON_UTILITIES_DATA_VARIANT_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
#include <Vadon/Utilities/Container/Box.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>
#include <vector>
#include <string>
#include <variant>
#include <unordered_map>
namespace Vadon::Utilities
{
	// Used by function bindings to indicate a void function
	enum class NoReturnValue : unsigned char {};

	struct VariantArray;
	struct VariantDictionary;

	using Variant = std::variant<std::monostate, int, float, bool, std::string,
		Utilities::Vector2, Utilities::Vector2i, Utilities::Vector3, Utilities::Vector3i, Utilities::Vector4,
		Vadon::Scene::ResourceHandle, Utilities::UUID,
		Utilities::Box<VariantArray>, Utilities::Box<VariantDictionary>, NoReturnValue>;

	struct VariantArray : public std::vector<Variant>
	{};

	// NOTE: for now we only support string keys
	struct VariantDictionary : public std::unordered_map<std::string, Variant>
	{
	};

	// Type remapping (when needing to convert an argument to a type compatible with the variant)
	// TODO: additional template args to provide both what we map from and what we map to?
	template<typename T, typename = void>
	struct VariantTypeMapping
	{
		using _Type = std::decay_t<T>;
	};

	template<typename T>
	using variant_type_mapping_t = typename VariantTypeMapping<T>::_Type;

	template<>
	struct VariantTypeMapping<std::string_view>
	{
		using _Type = std::string;
	};
}
#endif