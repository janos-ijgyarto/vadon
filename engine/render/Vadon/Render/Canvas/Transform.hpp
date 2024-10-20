#ifndef VADON_RENDER_CANVAS_TRANSFORM_HPP
#define VADON_RENDER_CANVAS_TRANSFORM_HPP
#include <Vadon/Render/Utilities/Vector.hpp>
namespace Vadon::Render::Canvas
{
	// FIXME: use a proper matrix?
	struct Transform
	{
		static Transform combine(const Transform& lhs, const Transform& rhs)
		{
			return Transform{ .position = lhs.position + rhs.position, .scale = lhs.scale * rhs.scale };
		}

		Vadon::Render::Vector2 position = Vadon::Render::Vector2_Zero;
		float scale = 1.0f;
	};
}
#endif