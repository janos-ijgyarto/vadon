#ifndef VADON_RENDER_CANVAS_PRIMITIVE_HPP
#define VADON_RENDER_CANVAS_PRIMITIVE_HPP
#include <Vadon/Render/Canvas/Material.hpp>
#include <Vadon/Render/Utilities/Color.hpp>
#include <Vadon/Render/Utilities/Vector.hpp>
#include <Vadon/Render/Utilities/Rectangle.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/SRV.hpp>
#include <vector>
namespace Vadon::Render::Canvas
{
	// TODO: some kind of utility object for colors?

	// FIXME: move each primitive into their own headers?

	// TODO: separate triangle primitive that only has colors?
	struct TriangleVertex
	{
		Vector2 position = { 0,0 };
		Vector2 uv = { 0, 0 };
		ColorRGBA color = Vadon::Math::Color_White;
	};

	struct Triangle
	{
		TriangleVertex points[3];
	};

	// Utility primitive for when we just want to have a colored quad
	// Optionally an "outline" instead of the filled rectangle
	struct Rectangle
	{
		Render::Rectangle dimensions = { .position = Vector2_Zero, .size = Vector2_One };
		ColorRGBA color = Vadon::Math::Color_White;
		bool filled = true; // TODO: replace with flags?
		float thickness = 1.0f;
	};

	// TODO: we could also require that textures used by the Canvas are "registered" with the system
	// and we then use an internal handle to reference it
	// This would allow adding safety checks to make sure the texture is compatible
	struct Texture
	{
		// TODO: any other info?
		SRVHandle srv;
	};

	// Utility primitive for when we just want a textured quad
	struct Sprite
	{
		Render::Rectangle dimensions = { .position = Vector2_Zero, .size = Vector2_One };
		Render::Rectangle uv_dimensions = { .position = Vector2_Zero, .size = Vector2_One };
		Vadon::Render::ColorRGBA color = Vadon::Math::Color_White;
	};

	using SpriteList = std::vector<Sprite>;
}
#endif