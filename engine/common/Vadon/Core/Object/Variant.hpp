#ifndef VADON_CORE_OBJECT_VARIANT_HPP
#define VADON_CORE_OBJECT_VARIANT_HPP
#include <Vadon/Utilities/Container/Box.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>
#include <vector>
#include <string>
#include <variant>
#include <unordered_map>
namespace Vadon::Core
{
	// Used by function bindings to indicate a void function
	enum class NoReturnValue : unsigned char {};

	struct VariantArray;
	struct VariantDictionary;

	using Variant = std::variant<std::monostate, int, float, bool, std::string, 
		Utilities::Vector2, Utilities::Vector2i, Utilities::Vector3, Utilities::Vector3i,
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