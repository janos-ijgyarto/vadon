#ifndef VADON_UTILITIES_MATH_VECTOR_HPP
#define VADON_UTILITIES_MATH_VECTOR_HPP
#include <glm/glm.hpp>
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
	using glm::distance;
	using glm::length;
	using glm::normalize;
}
#endif