#ifndef VADON_RENDER_UTILITIES_PROJECTION_HPP
#define VADON_RENDER_UTILITIES_PROJECTION_HPP
#include <Vadon/Utilities/Math/Matrix.hpp>
#include <glm/ext/matrix_clip_space.hpp>
namespace Vadon::Render
{
	// FIXME: move to header where others can use it, independent from matrices?
	enum class CoordinateSystem
	{
		LEFT_HAND,
		RIGHT_HAND
	};

	// NOTE: this corresponds to DirectX::XMMatrixPerspectiveFov (LH/RH determined by system)
	constexpr Utilities::Matrix4 create_directx_perspective_projection_matrix(float near, float far, float aspect_ratio, float fov_y, CoordinateSystem system = CoordinateSystem::LEFT_HAND)
	{
		const float half_fov_y = 0.5f * fov_y;
		const float sin_fov = std::sinf(half_fov_y);
		const float cos_fov = std::cosf(half_fov_y);

		const float height = cos_fov / sin_fov;
		const float width = height / aspect_ratio;

		if (system == CoordinateSystem::LEFT_HAND)
		{
			const float range = far / (far - near);

			return Utilities::Matrix4{
				{ width, 0, 0, 0 },
				{ 0, height, 0, 0 },
				{ 0, 0, range, 1.0f },
				{ 0, 0, -range * near, 0 }
			};
		}
		else
		{
			float range = far / (near - far);

			return Utilities::Matrix4{
				{ width, 0, 0, 0 },
				{ 0, height, 0, 0 },
				{ 0, 0, range, -1.0f },
				{ 0, 0, range * near, 0 }
			};
		}
	}

	// NOTE: this corresponds to DirectX::XMMatrixOrthographicOffCenter (LH/RH determined by system)
	constexpr Utilities::Matrix4 create_directx_orthographic_projection_matrix(float left, float right, float bottom, float top, float near, float far, CoordinateSystem system = CoordinateSystem::LEFT_HAND)
	{
		const float reciprocal_width = 1.0f / (right - left);
		const float reciprocal_height = 1.0f / (top - bottom);

		// FIXME: deduplicate more nicely, since only the range and the 4th row of the result differ?
		if (system == CoordinateSystem::LEFT_HAND)
		{
			const float range = 1.0f / (far - near);

			return Utilities::Matrix4{
				{ reciprocal_width + reciprocal_width, 0, 0, 0 },
				{ 0, reciprocal_height + reciprocal_height, 0, 0 },
				{ 0, 0, range, 0 },
				{ -((left + right) * reciprocal_width), -((top + bottom) * reciprocal_height), -range * near, 1.0f }
			};
		}
		else
		{
			const float range = 1.0f / (near - far);

			return Utilities::Matrix4{
				{ reciprocal_width + reciprocal_width, 0, 0, 0 },
				{ 0, reciprocal_height + reciprocal_height, 0, 0 },
				{ 0, 0, range, 0 },
				{ -((left + right) * reciprocal_width), -((top + bottom) * reciprocal_height), range * near, 1.0f }
			};
		}
	}

	// Uses the glm perspective matrix
	// NOTE: the result of this depends on the precompiler args passed to glm!
	constexpr Utilities::Matrix4 create_opengl_orthographic_projection_matrix(float near, float far, float aspect_ratio, float fov)
	{
		return glm::perspective(fov, aspect_ratio, near, far);
	}

	// Uses the glm orthographic matrix
	// NOTE: the result of this depends on the precompiler args passed to glm!
	constexpr Utilities::Matrix4 create_opengl_orthographic_projection_matrix(float left, float right, float bottom, float top, float near, float far)
	{
		return glm::ortho(left, right, bottom, top, near, far);
	}
}
#endif