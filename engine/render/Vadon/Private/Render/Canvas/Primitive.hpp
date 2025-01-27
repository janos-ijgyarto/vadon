#ifndef VADON_PRIVATE_RENDER_CANVAS_PRIMITIVE_HPP
#define VADON_PRIVATE_RENDER_CANVAS_PRIMITIVE_HPP
#include <Vadon/Render/Canvas/Primitive.hpp>
#include <Vadon/Private/Render/Utilities/Vector.hpp>
namespace Vadon::Private::Render::Canvas
{
	using ColorRGBA = Vadon::Render::Canvas::ColorRGBA;
	using PrimitiveBase = Vadon::Render::Canvas::PrimitiveBase;
	using Triangle = Vadon::Render::Canvas::Triangle;
	using Rectangle = Vadon::Render::Canvas::Rectangle;
	using Sprite = Vadon::Render::Canvas::Sprite;
	using SpriteList = Vadon::Render::Canvas::SpriteList;
	using TextInfo = Vadon::Render::Canvas::TextInfo;

	enum class PrimitiveType : uint32_t
	{
		TRIANGLE,
		RECTANGLE_FILL,
		RECTANGLE_OUTLINE,
		INVALID
	};

	// FIXME: could embed type index in vertex index, and bind the buffer with offsets if we exceed index limit
	struct PrimitiveInfo
	{
		PrimitiveType type = PrimitiveType::INVALID;
		uint32_t layer = 0;
		// TODO: other info?

		static constexpr uint32_t c_layer_index_width = 24;
		static constexpr uint32_t c_layer_index_mask = (1 << c_layer_index_width) - 1;

		constexpr uint32_t to_index() const
		{
			uint32_t index = Vadon::Utilities::to_integral(type) << c_layer_index_width;
			index |= (layer & c_layer_index_mask);

			return index;
		}

		void initialize(uint32_t data)
		{
			layer = data & c_layer_index_mask;
			type = Vadon::Utilities::to_enum<PrimitiveType>(data >> c_layer_index_width);
		}
	};

	struct alignas(16) PrimitiveVertex
	{
		Vector2 position = { 0, 0 };
		Vector2 uv = { 0, 0 };
	};

	struct alignas(16) PrimitiveRectangle
	{
		Vector2 position = { 0, 0 };
		Vector2 size = { 0, 0 };
	};

	// FIXME: compress primitive data members?
	struct alignas(16) TrianglePrimitiveData
	{
		uint32_t info = 0;
		uint32_t material = 0;
		float depth = 0.0f;
		uint32_t _padding = 0;

		PrimitiveVertex point_a;
		PrimitiveVertex point_b;
		PrimitiveVertex point_c;

		// TODO: color per-vertex?
		ColorRGBA color = { 0, 0, 0, 0 };
	};

	// TODO: rotatable rectangle?
	struct alignas(16) RectanglePrimitiveData
	{
		uint32_t info = 0;
		uint32_t material = 0;
		float depth = 0.0f;
		float thickness = 0.0f;

		PrimitiveRectangle dimensions;
		Vector2 uvs[4];

		ColorRGBA color = { 0, 0, 0, 0 };
	};
}
#endif