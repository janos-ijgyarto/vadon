#ifndef VADONDEMO_UI_COMPONENT_HPP
#define VADONDEMO_UI_COMPONENT_HPP
#include <Vadon/Render/Canvas/Item.hpp>
namespace VadonDemo::UI
{
	// TODO: implement some kind of logic which allows controlling "groups" of UI entities
	// e.g to show/hide all the elements of an entire widget
	struct Base
	{
		bool visible = true;

		Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
		Vadon::Utilities::Vector2 dimensions = Vadon::Utilities::Vector2_Zero;
		float z_order = 0.0f;

		// TODO: position and sizing style, whether to use absolute coords or relative to screen size

		// FIXME: decouple from visualization
		// Could create Render component/system which can be used by View and UI to visualize their content
		// e.g UI system takes UI components and feeds draw data into Render component
		Vadon::Render::Canvas::ItemHandle canvas_item;

		static void register_component();
	};

	struct Frame
	{
		Vadon::Utilities::Vector3 outline_color = Vadon::Utilities::Vector3_One;
		float outline_thickness = 1.0f;

		Vadon::Utilities::Vector3 fill_color = Vadon::Utilities::Vector3_One;
		bool fill_enable = false;

		// TODO: draw anchor, to decide whether to draw relative to center or top left corner

		static void register_component();
	};

	struct Text
	{
		std::string text;
		Vadon::Utilities::Vector3 color = Vadon::Utilities::Vector3_One;
		Vadon::Utilities::Vector2 offset = Vadon::Utilities::Vector2_Zero;
		// TODO: font, text size, etc.
		// TODO: alignment (left vs center)

		static void register_component();
	};

	struct Selectable
	{
		// TODO: more advanced signal/callback system?
		std::string clicked_key;

		static void register_component();
	};
}
#endif