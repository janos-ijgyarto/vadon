#ifndef VADON_UTILITIES_ENUM_ENUMCLASSBITFLAG_HPP
#define VADON_UTILITIES_ENUM_ENUMCLASSBITFLAG_HPP
#include <Vadon/Utilities/Enum/EnumClass.hpp>

namespace Vadon::Utilities
{
	template <typename T>
	struct EnableEnumBitwiseOperators : public std::false_type
	{};
}

template <typename T>
constexpr std::enable_if_t<Vadon::Utilities::EnableEnumBitwiseOperators<T>::value, T> operator&(T lhs, T rhs)
{
	return Vadon::Utilities::to_enum<T>(Vadon::Utilities::to_integral(lhs) & Vadon::Utilities::to_integral(rhs));
}

template <typename T>
constexpr std::enable_if_t<Vadon::Utilities::EnableEnumBitwiseOperators<T>::value, T> operator|(T lhs, T rhs)
{
	return Vadon::Utilities::to_enum<T>(Vadon::Utilities::to_integral(lhs) | Vadon::Utilities::to_integral(rhs));
}

template <typename T>
constexpr std::enable_if_t<Vadon::Utilities::EnableEnumBitwiseOperators<T>::value, T> operator^(T lhs, T rhs)
{
	return Vadon::Utilities::to_enum<T>(Vadon::Utilities::to_integral(lhs) ^ Vadon::Utilities::to_integral(rhs));
}

template <typename T>
constexpr std::enable_if_t<Vadon::Utilities::EnableEnumBitwiseOperators<T>::value, T> operator<<(T lhs, size_t rhs)
{
	return Vadon::Utilities::to_enum<T>(Vadon::Utilities::to_integral(lhs) << rhs);
}

template <typename T>
constexpr std::enable_if_t<Vadon::Utilities::EnableEnumBitwiseOperators<T>::value, T> operator>>(T lhs, size_t rhs)
{
	return Vadon::Utilities::to_enum<T>(Vadon::Utilities::to_integral(lhs) >> rhs);
}

template <typename T>
constexpr std::enable_if_t<Vadon::Utilities::EnableEnumBitwiseOperators<T>::value, T> operator~(T lhs)
{
	return Vadon::Utilities::to_enum<T>(~Vadon::Utilities::to_integral(lhs));
}

template <typename T>
std::enable_if_t<Vadon::Utilities::EnableEnumBitwiseOperators<T>::value, T>& operator|=(T& lhs, T rhs)
{
	lhs = Vadon::Utilities::to_enum<T>(Vadon::Utilities::to_integral(lhs) | Vadon::Utilities::to_integral(rhs));
	return lhs;
}

template <typename T>
std::enable_if_t<Vadon::Utilities::EnableEnumBitwiseOperators<T>::value, T>& operator&=(T& lhs, T rhs)
{
	lhs = Vadon::Utilities::to_enum<T>(Vadon::Utilities::to_integral(lhs) & Vadon::Utilities::to_integral(rhs));
	return lhs;
}

template <typename T>
std::enable_if_t<Vadon::Utilities::EnableEnumBitwiseOperators<T>::value, T>& operator^=(T& lhs, T rhs)
{
	lhs = Vadon::Utilities::to_enum<T>(Vadon::Utilities::to_integral(lhs) ^ Vadon::Utilities::to_integral(rhs));
	return lhs;
}

template <typename T>
std::enable_if_t<Vadon::Utilities::EnableEnumBitwiseOperators<T>::value, T>& operator<<=(T& lhs, size_t rhs)
{
	lhs = Vadon::Utilities::to_enum<T>(Vadon::Utilities::to_integral(lhs) << rhs);
	return lhs;
}

template <typename T>
std::enable_if_t<Vadon::Utilities::EnableEnumBitwiseOperators<T>::value, T>& operator>>=(T& lhs, size_t rhs)
{
	lhs = Vadon::Utilities::to_enum<T>(Vadon::Utilities::to_integral(lhs) >> rhs);
	return lhs;
}
#endif