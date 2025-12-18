#include <Vadon/Private/Render/Canvas/CommandBuffer.hpp>

#include <Vadon/Private/Render/Canvas/Primitive.hpp>

#include <Vadon/Render/Canvas/Layer.hpp>

#include <Vadon/Utilities/Enum/EnumClass.hpp>

namespace Vadon::Render::Canvas
{
	void CommandBuffer::draw_triangle(const Vadon::Render::Canvas::Triangle& triangle)
	{
		using namespace Vadon::Private::Render::Canvas;

		TrianglePrimitiveData* triangle_data = allocate<TrianglePrimitiveData>();

		triangle_data->info = Vadon::Utilities::to_integral(CommandType::TRIANGLE);
		for (size_t vertex = 0; vertex < 3; ++vertex)
		{
			triangle_data->points[vertex] = PrimitiveVertex{ .position = triangle.points[vertex].position, .uv = triangle.points[vertex].uv };
			triangle_data->color[static_cast<Vector3u::length_type>(vertex)] = triangle.points[vertex].color.value;
		}
	}

	void CommandBuffer::draw_rectangle(const Vadon::Render::Canvas::Rectangle& rectangle)
	{
		using namespace Vadon::Private::Render::Canvas;

		const uint32_t flags = rectangle.filled ? Vadon::Utilities::to_integral(RectanglePrimitiveFlags::FILLED) : 0;

		RectanglePrimitiveData* rectangle_primitive = allocate<RectanglePrimitiveData>();
		rectangle_primitive->info = CommandInfo{ Vadon::Utilities::to_integral(CommandType::RECTANGLE), flags }.to_uint();
		rectangle_primitive->rotation = rectangle.rotation;
		rectangle_primitive->dimensions.position = rectangle.dimensions.position;
		rectangle_primitive->dimensions.size = rectangle.dimensions.size;
		rectangle_primitive->thickness = rectangle.thickness;
		rectangle_primitive->color = rectangle.color.value;
	}

	void CommandBuffer::draw_circle(const Vadon::Render::Canvas::Circle& circle)
	{
		using namespace Vadon::Private::Render::Canvas;

		CirclePrimitiveData* circle_primitive = allocate<CirclePrimitiveData>();
		circle_primitive->info = Vadon::Utilities::to_integral(CommandType::CIRCLE);
		circle_primitive->position = circle.position;
		circle_primitive->radius = circle.radius;
		circle_primitive->color = circle.color.value;

		circle_primitive->uv_dimensions.position = circle.uv_dimensions.position;
		circle_primitive->uv_dimensions.size = circle.uv_dimensions.size;
	}

	void CommandBuffer::draw_sprite(const Vadon::Render::Canvas::Sprite& sprite)
	{
		using namespace Vadon::Private::Render::Canvas;

		SpritePrimitiveData* sprite_primitive = allocate<SpritePrimitiveData>();
		sprite_primitive->info = Vadon::Utilities::to_integral(CommandType::SPRITE);
		sprite_primitive->dimensions.position = sprite.dimensions.position;
		sprite_primitive->dimensions.size = sprite.dimensions.size;
		sprite_primitive->rotation = sprite.rotation;
		sprite_primitive->uv_dimensions.position = sprite.uv_dimensions.position;
		sprite_primitive->uv_dimensions.size = sprite.uv_dimensions.size;
		sprite_primitive->color = sprite.color.value;
	}

	void CommandBuffer::add_batch_draw(const Vadon::Render::Canvas::BatchDrawCommand& batch_command)
	{
		using namespace Vadon::Private::Render::Canvas;

		push_data(Vadon::Utilities::to_integral(CommandType::BATCH));
		push_data(batch_command);
	}

	void CommandBuffer::set_texture(const Vadon::Render::Canvas::Texture& texture)
	{
		using namespace Vadon::Private::Render::Canvas;

		push_data(Vadon::Utilities::to_integral(CommandType::SET_TEXTURE));
		push_data(texture);
	}

	void CommandBuffer::set_material(Vadon::Render::Canvas::MaterialHandle material_handle)
	{
		using namespace Vadon::Private::Render::Canvas;

		push_data(Vadon::Utilities::to_integral(CommandType::SET_MATERIAL));
		push_data(material_handle);
	}

	void CommandBuffer::set_render_state(const Vadon::Render::Canvas::RenderState& render_state)
	{
		using namespace Vadon::Private::Render::Canvas;

		push_data(Vadon::Utilities::to_integral(CommandType::SET_RENDER_STATE));
		push_data(render_state);
	}
}