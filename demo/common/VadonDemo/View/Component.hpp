#ifndef VADONDEMO_VIEW_COMPONENT_HPP
#define VADONDEMO_VIEW_COMPONENT_HPP
#include <VadonDemo/View/Resource.hpp>
#include <Vadon/Render/Canvas/Item.hpp>
namespace VadonDemo::View
{
	// Adding this component means the Entity needs to be visualized
	struct ViewComponent
	{
		ViewResourceHandle resource;
		float z_order = 0.0f;
		// TODO: flag for visibility?

		Vadon::Render::Canvas::Transform prev_transform;
		Vadon::Render::Canvas::Transform current_transform;
		Vadon::Render::Canvas::ItemHandle canvas_item;

		static void register_component();
	};
}
#endif