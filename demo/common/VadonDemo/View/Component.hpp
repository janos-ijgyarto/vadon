#ifndef VADONDEMO_VIEW_COMPONENT_HPP
#define VADONDEMO_VIEW_COMPONENT_HPP
#include <VadonDemo/View/Resource.hpp>
#include <Vadon/Render/Canvas/Transform.hpp>
namespace VadonDemo::View
{
	// Adding this component means the Entity needs to be visualized
	struct ViewComponent
	{
		Vadon::Render::Canvas::Transform prev_transform;
		Vadon::Render::Canvas::Transform current_transform;

		ViewResourceHandle resource;

		static void register_component();
	};
}
#endif