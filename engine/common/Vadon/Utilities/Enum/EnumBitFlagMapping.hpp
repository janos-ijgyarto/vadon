#ifndef VADON_UTILITIES_ENUM_ENUMBITFLAGMAPPING_HPP
#define VADON_UTILITIES_ENUM_ENUMBITFLAGMAPPING_HPP
#include <array>
#include <utility>
namespace Vadon::Utilities
{
	// NOTE: Matches ConstexprMap, but instead of lookups, it does bitwise conversions
	template<typename FirstEnum, typename SecondEnum, std::size_t Size>
	struct EnumBitFlagMapping
	{
		using Pair = std::pair<FirstEnum, SecondEnum>;
		using PairArray = std::array<Pair, Size>;

		using FirstEnumUnderlying = std::underlying_type_t<FirstEnum>;
		using SecondEnumUnderlying = std::underlying_type_t<SecondEnum>;

		PairArray data;

		constexpr SecondEnum convert(FirstEnum input) const
		{
			static_assert(std::is_enum_v<FirstEnum>, "Must be enum");
			static_assert(std::is_enum_v<SecondEnum>, "Must be enum");

			FirstEnumUnderlying input_underlying = static_cast<FirstEnumUnderlying>(input);
			SecondEnumUnderlying output_underlying = SecondEnumUnderlying{ 0 };
			for (size_t index = 0; index < Size; ++index)
			{
				const Pair& current_pair = data[index];
				if (static_cast<FirstEnumUnderlying>(current_pair.first) & input_underlying)
				{
					output_underlying |= static_cast<SecondEnumUnderlying>(current_pair.second);
				}
			}

			return static_cast<SecondEnum>(output_underlying);
		}

		constexpr FirstEnum reverse_convert(SecondEnum input) const
		{
			static_assert(std::is_enum_v<FirstEnum>, "Must be enum");
			static_assert(std::is_enum_v<SecondEnum>, "Must be enum");

			SecondEnumUnderlying input_underlying = static_cast<SecondEnumUnderlying>(input);
			FirstEnumUnderlying output_underlying = FirstEnumUnderlying{ 0 };
			for (size_t index = 0; index < Size; ++index)
			{
				const Pair& current_pair = data[index];
				if (static_cast<SecondEnumUnderlying>(current_pair.second) & input_underlying)
				{
					output_underlying |= static_cast<FirstEnumUnderlying>(current_pair.first);
				}
			}

			return static_cast<FirstEnum>(output_underlying);
		}
	};

	template<typename FirstEnum, typename SecondEnum, std::size_t Size>
	inline constexpr EnumBitFlagMapping<FirstEnum, SecondEnum, Size> create_bit_flag_mapping(const std::pair<FirstEnum, SecondEnum> (&data)[Size])
	{
		EnumBitFlagMapping<FirstEnum, SecondEnum, Size> mapping;
		std::copy(std::begin(data), std::begin(data) + Size, mapping.data.data());

		return mapping;
	}
}
#endif