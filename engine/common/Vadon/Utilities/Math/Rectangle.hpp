#ifndef VADON_UTILITIES_MATH_RECTANGLE_HPP
#define VADON_UTILITIES_MATH_RECTANGLE_HPP
#include <Vadon/Utilities/Math/Vector.hpp>
namespace Vadon::Utilities
{
	template<typename VEC2_TYPE>
	struct RectangleBase
	{
		VEC2_TYPE position{ 0, 0 }; // Top left
		VEC2_TYPE size{ 0, 0 }; // Width and height

		// TODO: utility functions!

		bool operator==(const RectangleBase<VEC2_TYPE>& other) const { return (position == other.position) && (size == other.size); }
		bool operator!=(const RectangleBase<VEC2_TYPE>& other) const { return (position != other.position) || (size != other.size); }
	};

	using Rectangle = RectangleBase<Vector2>;
	using RectangleInt = RectangleBase<Vector2i>;
}
#endif