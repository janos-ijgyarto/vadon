#ifndef VADON_UTILITIES_CONTAINER_MAP_CONSTEXPRMAP_HPP
#define VADON_UTILITIES_CONTAINER_MAP_CONSTEXPRMAP_HPP
#include <array>
#include <Vadon/Utilities/Debugging/Assert.hpp>
namespace Vadon::Utilities
{
	// NOTE: inspired by Jason Turner's constexpr map video
	template<typename Key, typename Value, std::size_t Size>
	struct ConstexprMap
	{
		using Pair = std::pair<Key, Value>;
		std::array<Pair, Size> data;

		[[nodiscard]] constexpr Value at(const Key& key) const
		{
			for (const Pair& current_pair : data)
			{
				if (current_pair.first == key)
				{
					return current_pair.second;
				}
			}

			VADON_UNREACHABLE;
		}

		[[nodiscard]] constexpr Value reverse_at(const Value& value) const
		{
			for (const Pair& current_pair : data)
			{
				if (current_pair.second == value)
				{
					return current_pair.first;
				}
			}

			VADON_UNREACHABLE;
		}
	};

	template<typename Key, typename Value, std::size_t Size>
	inline constexpr ConstexprMap<Key, Value, Size> create_constexpr_map(const std::pair<Key, Value>(&data)[Size])
	{
		ConstexprMap<Key, Value, Size> map;
		std::copy(std::begin(data), std::begin(data) + Size, map.data.data());

		return map;
	}
}
#endif