#ifndef VADON_UTILITIES_DATA_VARIANTDICTIONARY_HPP
#define VADON_UTILITIES_DATA_VARIANTDICTIONARY_HPP
#include <Vadon/Utilities/Data/VariantBase.hpp>
#include <unordered_map>
namespace Vadon::Utilities
{
	// NOTE: for now we only support string keys
	using VariantUnorderedMap = std::unordered_map<std::string, Variant>;
	struct VariantDictionary
	{
		VariantUnorderedMap data;

		bool operator==(const VariantDictionary& other) const
		{
			for (const auto& current_entry : data)
			{
				auto other_it = other.data.find(current_entry.first);
				if(other_it == other.data.end())
				{
					return false;
				}

				if (other_it->second != current_entry.second)
				{
					return false;
				}
			}

			return true;
		}

		bool operator!=(const VariantDictionary& other) const
		{
			return (*this == other) == false;
		}
	};

	template<>
	struct VariantTypeTrait<VariantDictionary>
	{
		static Variant to_variant(const VariantDictionary& dictionary)
		{
			return Variant(Box(dictionary));
		}

		static VariantDictionary from_variant(const Variant& variant)
		{
			return *std::get<Box<VariantDictionary>>(variant);
		}
	};
}
#endif