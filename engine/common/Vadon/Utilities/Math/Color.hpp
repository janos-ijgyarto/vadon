#ifndef VADON_UTILITIES_MATH_COLOR_HPP
#define VADON_UTILITIES_MATH_COLOR_HPP
#include <glm/vec4.hpp>
#include <glm/packing.hpp>
#include <glm/gtx/vec_swizzle.hpp>

namespace Vadon::Utilities
{
	using ColorVector = glm::vec4;

	struct ColorRGBA
	{
		uint32_t value = 0;

		static ColorVector to_rgba_vector(const ColorRGBA& color) 
		{ 
			// Have to swizzle because GLM flips the order
			return glm::wzyx(glm::unpackUnorm4x8(color.value));
		}

		static ColorRGBA from_rgba_vector(const ColorVector& vector) 
		{ 
			// Have to swizzle because GLM flips the order
			return ColorRGBA(glm::packUnorm4x8(glm::wzyx(vector)));
		}

		bool operator==(const ColorRGBA& other) const { return value == other.value; }
		bool operator!=(const ColorRGBA& other) const { return value != other.value; }
	};

	constexpr ColorRGBA Color_White = ColorRGBA{ 0xFFFFFFFF };
	constexpr ColorRGBA Color_Black = ColorRGBA{ 0x0 };
}
#endif