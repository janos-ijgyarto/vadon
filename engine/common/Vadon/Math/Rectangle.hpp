#ifndef VADON_MATH_RECTANGLE_HPP
#define VADON_MATH_RECTANGLE_HPP
#include <Vadon/Math/Vector.hpp>
namespace Vadon::Math
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