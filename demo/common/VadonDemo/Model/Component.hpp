#ifndef VADONDEMO_MODEL_COMPONENT_HPP
#define VADONDEMO_MODEL_COMPONENT_HPP
#include <Vadon/ECS/Component/Registry.hpp>
#include <Vadon/Render/Canvas/Item.hpp>
namespace VadonDemo::Model
{
	struct Transform2D
	{
		Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
		float rotation = 0;
		float scale = 1.0f;

		Vadon::Utilities::Vector2 global_position = Vadon::Utilities::Vector2_Zero;

		static void register_component();
	};

	struct CanvasComponent
	{
		Vadon::Utilities::Vector3 color = Vadon::Utilities::Vector3_One;
		float scale = 0.0f;

		Vadon::Render::Canvas::ItemHandle canvas_handle;

		static void register_component();
	};

	struct Celestial
	{
		bool is_star = false;
		float angular_velocity = 1.0f;
		float radius = 0.0f;

		static void register_component();
	};
}
#endif