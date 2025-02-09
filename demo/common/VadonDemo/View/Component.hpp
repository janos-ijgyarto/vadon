#ifndef VADONDEMO_VIEW_COMPONENT_HPP
#define VADONDEMO_VIEW_COMPONENT_HPP
#include <VadonDemo/View/Resource.hpp>
#include <Vadon/Render/Canvas/Item.hpp>
namespace VadonDemo::View
{
	// FIXME: split functionality between "View" and "Render" (or "Canvas") components
	// View refers to something from the Model that needs visualization, uses interpolation, etc.
	// Render has a Canvas item handle, lets other systems fill it with draw commands
	// This will allow View and UI to both use the same interfaces for visualization, without knowing about each other
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