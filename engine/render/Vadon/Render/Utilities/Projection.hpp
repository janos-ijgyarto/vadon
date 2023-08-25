#ifndef VADON_RENDER_UTILITIES_PROJECTION_HPP
#define VADON_RENDER_UTILITIES_PROJECTION_HPP
#include <Vadon/Utilities/Math/Matrix.hpp>
namespace Vadon::Render
{
	constexpr Utilities::Matrix4 create_perspective_projection_matrix(float near, float far, float aspect_ratio, float fov)
	{
		return Utilities::Matrix4 {
			{ 1.0f / (aspect_ratio * std::tanf(fov / 2.0f)), 0, 0, 0 },
			{ 0, 1.0f / std::tanf(fov / 2.0f), 0, 0 },
			{ 0, 0, -((far + near) / (far - near)), -1.0f },
			{ 0, 0, -((2.0f * far * near) / (far - near)), 0 }
		};
	}

	constexpr Utilities::Matrix4 create_orthographic_projection_matrix(float left, float right, float bottom, float top, float near, float far)
	{
		// https://en.wikipedia.org/wiki/Orthographic_projection
		return Utilities::Matrix4 {
			{ 2.0f / (right - left), 0, 0, 0 },
			{ 0, 2.0f / (top - bottom), 0, 0 },
			{ 0, 0, -2.0f / (far - near), 0 },
			{ -((right + left) / (right - left)), -((top + bottom) / (top - bottom)), -((far + near) / (far - near)), 1.0f }
		};
	}
}
#endif