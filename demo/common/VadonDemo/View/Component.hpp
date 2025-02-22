#ifndef VADONDEMO_VIEW_COMPONENT_HPP
#define VADONDEMO_VIEW_COMPONENT_HPP
#include <VadonDemo/View/Resource.hpp>
namespace VadonDemo::View
{
	struct ViewComponent
	{
		ViewResourceHandle resource;

		Vadon::Render::Canvas::Transform prev_transform;
		Vadon::Render::Canvas::Transform current_transform;

		static void register_component();
	};
}
#endif