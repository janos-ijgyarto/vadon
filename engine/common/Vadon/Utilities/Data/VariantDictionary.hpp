#ifndef VADON_UTILITIES_DATA_VARIANTDICTIONARY_HPP
#define VADON_UTILITIES_DATA_VARIANTDICTIONARY_HPP
#include <Vadon/Utilities/Data/VariantBase.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
#include <unordered_map>
namespace Vadon::Utilities
{
	// NOTE: for now we only support string keys
	using VariantUnorderedMap = std::unordered_map<std::string, Variant>;
	struct VariantDictionary
	{
		VariantUnorderedMap data;
		TypeID data_type;

		bool operator==(const VariantDictionary& /*other*/) const
		{
			// TODO!!!
			return false;
		}

		bool operator!=(const VariantDictionary& /*other*/) const
		{
			// TODO!!!
			return true;
		}
	};
}
#endif