#ifndef VADONDEMO_VIEW_COMPONENT_HPP
#define VADONDEMO_VIEW_COMPONENT_HPP
#include <VadonDemo/View/Resource.hpp>
namespace VadonDemo::View
{
	struct ViewComponent
	{
		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		ViewResourceID resource;
		bool dirty = false; // FIXME: replace with editor-specific "tag component" implementation!

		Vadon::Render::Canvas::Transform prev_transform;
		Vadon::Render::Canvas::Transform current_transform;

		static void register_component();
	};
}
#endif