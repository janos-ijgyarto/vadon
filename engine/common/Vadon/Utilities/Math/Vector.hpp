#ifndef VADON_UTILITIES_MATH_VECTOR_HPP
#define VADON_UTILITIES_MATH_VECTOR_HPP
// TODO: separate types and functions, create forward declaration header
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/vector_relational.hpp>
#include <glm/geometric.hpp>
namespace Vadon::Utilities
{
	using Vector2 = glm::vec2;
	using Vector3 = glm::vec3;
	using Vector4 = glm::vec4;

	using Vector2i = glm::ivec2;
	using Vector3i = glm::ivec3;
	using Vector4i = glm::ivec4;

	using Vector2u = glm::uvec2;
	using Vector3u = glm::uvec3;
	using Vector4u = glm::uvec4;

	// TODO: additional constants?
	constexpr Vector2 Vector2_Zero = Vector2{ 0, 0 };
	constexpr Vector2 Vector2_One = Vector2{ 1, 1 };

	constexpr Vector3 Vector3_Zero = Vector3{ 0, 0, 0 };
	constexpr Vector3 Vector3_One = Vector3{ 1, 1, 1 };

	constexpr Vector4 Vector4_Zero = Vector4{ 0, 0, 0, 0 };
	constexpr Vector4 Vector4_One = Vector4{ 1, 1, 1, 1 };

	// TODO: organize these?
	using glm::any;
	using glm::all;
	using glm::distance;
	using glm::length;
	using glm::normalize;
	using glm::abs;
	using glm::dot;
	using glm::min;
	using glm::max;
	using glm::clamp;
	using glm::lessThan;
	using glm::greaterThan;

	template<typename T>
	inline constexpr float length_squared(T const& x)
	{
		return dot(x, x);
	}

	template<typename T>
	inline constexpr float distance_squared(T const& x, T const& y)
	{
		T temp = x - y;
		return dot(temp, temp);
	}
}
#endif