#ifndef VADONDEMO_MODEL_COMPONENT_HPP
#define VADONDEMO_MODEL_COMPONENT_HPP
#include <Vadon/ECS/Component/Registry.hpp>
#include <Vadon/Render/Canvas/Item.hpp>
#include <Vadon/Utilities/Enum/EnumClass.hpp>
namespace VadonDemo::Model
{
	struct Transform2D
	{
		Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
		float rotation = 0;
		float scale = 1.0f;

		Vadon::Utilities::Vector2 global_position = Vadon::Utilities::Vector2_Zero;
		Vadon::Utilities::Vector2 prev_position = Vadon::Utilities::Vector2_Zero;

		static void register_component();
	};

	// FIXME: could replace with system where we register procedures that generate the primitive data based on Render obj state
	enum class RenderObjectType
	{
		TRIANGLE,
		BOX,
		DIAMOND
		// TODO: other shapes!
	};

	struct CanvasComponent
	{
		int32_t type = Vadon::Utilities::to_integral(RenderObjectType::TRIANGLE); // FIXME: implement a way to do enums, list of selectable options, etc.
		Vadon::Utilities::Vector3 color = Vadon::Utilities::Vector3_One;
		
		int32_t render_handle = -1;

		static void register_component();
	};

	struct Celestial
	{
		bool is_star = false; // NOTE: currently does nothing!
		float angular_velocity = 1.0f;
		float radius = 0.0f;

		static void register_component();
	};
}
#endif