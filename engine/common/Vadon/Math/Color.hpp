#ifndef VADON_MATH_COLOR_HPP
#define VADON_MATH_COLOR_HPP
#include <glm/vec4.hpp>
#include <glm/packing.hpp>

namespace Vadon::Math
{
	using ColorVector = glm::vec4;

	struct ColorRGBA
	{
		uint32_t value = 0;

		static ColorVector to_rgba_vector(const ColorRGBA& color) 
		{
			return glm::unpackUnorm4x8(color.value);
		}

		static ColorRGBA from_rgba_vector(const ColorVector& vector) 
		{
			return ColorRGBA(glm::packUnorm4x8(vector));
		}

		static ColorRGBA multiply_colors(const ColorRGBA& lhs, const ColorRGBA& rhs)
		{
			return from_rgba_vector(to_rgba_vector(lhs) * to_rgba_vector(rhs));
		}

		bool operator==(const ColorRGBA& other) const { return value == other.value; }
		bool operator!=(const ColorRGBA& other) const { return value != other.value; }
	};

	constexpr ColorRGBA Color_White = ColorRGBA{ 0xFFFFFFFF };
	constexpr ColorRGBA Color_Black = ColorRGBA{ 0x0 };
}
#endif