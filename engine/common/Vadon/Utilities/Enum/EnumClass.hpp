#ifndef VADON_UTILITIES_ENUM_ENUMCLASS_HPP
#define VADON_UTILITIES_ENUM_ENUMCLASS_HPP
#include <type_traits>
namespace Vadon::Utilities
{
	template <typename T>
	constexpr auto to_integral(T value)
	{
		// Add a check just in case somebody tried to call this on a non-enum for some reason
		if constexpr (std::is_enum_v<T>)
		{
			return static_cast<std::underlying_type_t<T>>(value);
		}
		else
		{
			return value;
		}
	}

	template<typename T>
	constexpr std::enable_if_t<std::is_enum_v<T>, bool> to_bool(T value)
	{
		return to_integral(value) != 0;
	}

	template<typename T>
	constexpr std::enable_if_t<std::is_enum_v<T>, T> to_enum(std::underlying_type_t<T> value)
	{
		return static_cast<T>(value);
	}

	// NOTE: this version is meant to be used if we do not match underlying type
	// Assumes the conversion is still valid
	template<typename T, typename V>
	constexpr std::enable_if_t<std::is_enum_v<T>, T> convert_to_enum(V value)
	{
		return static_cast<T>(value);
	}
}
#endif