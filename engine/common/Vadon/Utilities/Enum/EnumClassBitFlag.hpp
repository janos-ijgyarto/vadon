#ifndef VADON_UTILITIES_ENUM_ENUMCLASSBITFLAG_HPP
#define VADON_UTILITIES_ENUM_ENUMCLASSBITFLAG_HPP
#include <Vadon/Utilities/Enum/EnumClass.hpp>

#define VADON_START_BITMASK_SWITCH(_bitmask) \
for (uint64_t current_bit = 1; Vadon::Utilities::to_integral(_bitmask) >= current_bit; current_bit *= 2) \
if ((Vadon::Utilities::to_integral(_bitmask) & current_bit) != 0) \
switch (static_cast<decltype(_bitmask)>(current_bit))

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

#define VADON_ENABLE_BITWISE_OPERATORS(_type) namespace Vadon::Utilities { template<> struct EnableEnumBitwiseOperators<_type> : public std::true_type{}; }
#endif