#ifndef VADON_RENDER_CANVAS_PRIMITIVE_HPP
#define VADON_RENDER_CANVAS_PRIMITIVE_HPP
#include <Vadon/Render/Canvas/Material.hpp>
#include <Vadon/Render/Utilities/Vector.hpp>
#include <Vadon/Render/Utilities/Rectangle.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Resource.hpp>
#include <vector>
namespace Vadon::Render::Canvas
{
	// FIXME: move these into their own headers?
	using ColorRGBA = Vector4;

	struct PrimitiveBase
	{
		ColorRGBA color = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
		MaterialHandle material;
	};

	struct Vertex
	{
		Vector2 position = { 0,0 };
		Vector2 uv = { 0, 0 };
	};

	// TODO: textured triangles?
	struct Triangle : public PrimitiveBase
	{
		Vertex point_a;
		Vertex point_b;
		Vertex point_c;
	};

	// TODO: merge rectangle and sprite somehow?
	struct Rectangle : public PrimitiveBase
	{
		Render::Rectangle dimensions;
		bool filled = true;
		float thickness = 1.0f;
	};

	struct Sprite : public PrimitiveBase
	{
		Render::Rectangle dimensions;
		Render::Rectangle uv_dimensions;
		ResourceViewHandle texture_handle;
	};

	using SpriteList = std::vector<Sprite>;

	// NOTE: only used as utility object to convert text to sprites
	struct TextInfo : public PrimitiveBase
	{
		Vector2 position;
	};
}
#endif