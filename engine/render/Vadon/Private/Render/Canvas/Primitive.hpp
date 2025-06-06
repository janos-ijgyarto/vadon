#ifndef VADON_PRIVATE_RENDER_CANVAS_PRIMITIVE_HPP
#define VADON_PRIVATE_RENDER_CANVAS_PRIMITIVE_HPP
#include <Vadon/Render/Canvas/Primitive.hpp>
#include <Vadon/Private/Render/Utilities/Vector.hpp>
namespace Vadon::Private::Render::Canvas
{
	using Triangle = Vadon::Render::Canvas::Triangle;
	using Rectangle = Vadon::Render::Canvas::Rectangle;
	using Texture = Vadon::Render::Canvas::Texture;
	using Sprite = Vadon::Render::Canvas::Sprite;
	using SpriteList = Vadon::Render::Canvas::SpriteList;

	enum class PrimitiveType : uint32_t
	{
		TRIANGLE,
		RECTANGLE_FILL,
		RECTANGLE_OUTLINE,
		SPRITE,
		INVALID
	};

	// TODO: might be easier to just use a union?
	struct PrimitiveInfo
	{
		PrimitiveType type = PrimitiveType::INVALID;
		uint32_t layer = 0;
		uint32_t material = 0;

		static constexpr uint32_t c_layer_index_width = 8;
		static constexpr uint32_t c_material_index_width = 16;

		static constexpr uint32_t c_layer_index_mask = (1 << c_layer_index_width) - 1;
		static constexpr uint32_t c_material_index_mask = (1 << c_material_index_width) - 1;

		constexpr uint32_t to_index() const
		{
			uint32_t index = layer & c_layer_index_mask;

			index |= (material & c_material_index_mask) << c_layer_index_width;
			index |= Vadon::Utilities::to_integral(type) << (c_layer_index_width + c_material_index_width);

			return index;
		}
	};

	struct alignas(16) PrimitiveVertex
	{
		Vector2 position = { 0, 0 };
		Vector2 uv = { 0, 0 };
	};

	struct alignas(16) TrianglePrimitiveData
	{
		uint32_t info = 0;
		Vector3u color = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
		PrimitiveVertex points[3];
	};

	struct alignas(16) PrimitiveRectangle
	{
		Vector2 position = { 0, 0 };
		Vector2 size = { 0, 0 };
	};

	struct alignas(16) RectanglePrimitiveData
	{
		uint32_t info = 0;
		uint32_t color = 0xFFFFFFFF;
		float thickness = 1.0f;
		uint32_t _padding = 0;

		PrimitiveRectangle dimensions;
	};

	struct alignas(16) SpritePrimitiveData
	{
		uint32_t info = 0;
		uint32_t color = 0xFFFFFFF;
		Vector2u _padding = { 0, 0 };

		PrimitiveRectangle dimensions;
		PrimitiveRectangle uv_dimensions;
	};
}
#endif