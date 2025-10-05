#ifndef VADON_MATH_MATRIX_HPP
#define VADON_MATH_MATRIX_HPP
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
namespace Vadon::Math
{
	using Matrix2 = glm::mat2x2;
	using Matrix3 = glm::mat3x3;
	using Matrix4 = glm::mat4x4;

	// TODO: additional constants?
	constexpr Matrix4 Matrix2_Zeros = Matrix4();
	constexpr Matrix4 Matrix2_Identity = Matrix4(1.0f);
}
#endif