#ifndef VADONDEMO_UI_COMPONENT_HPP
#define VADONDEMO_UI_COMPONENT_HPP
#include <Vadon/Render/Canvas/Item.hpp>
#include <Vadon/Render/Utilities/Color.hpp>
namespace VadonDemo::UI
{
	// TODO: implement some kind of logic which allows controlling "groups" of UI entities
	// e.g to show/hide all the elements of an entire widget
	struct Base
	{
		bool enabled = true;

		Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
		Vadon::Utilities::Vector2 dimensions = Vadon::Utilities::Vector2_Zero;

		// TODO: position and sizing style, whether to use absolute coords or relative to screen size

		bool dirty = false; // FIXME: replace with editor-specific "tag component" implementation!

		static void register_component();
	};

	struct Frame
	{
		Vadon::Render::ColorRGBA outline_color = Vadon::Utilities::Color_White;
		float outline_thickness = 1.0f;

		Vadon::Render::ColorRGBA fill_color = Vadon::Utilities::Color_White;
		bool fill_enable = false;

		// TODO: draw anchor, to decide whether to draw relative to center or top left corner

		static void register_component();
	};

	struct Text
	{
		std::string text;
		Vadon::Render::ColorRGBA color = Vadon::Utilities::Color_White;
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