#ifndef VADON_PRIVATE_RENDER_CANVAS_LAYER_HPP
#define VADON_PRIVATE_RENDER_CANVAS_LAYER_HPP
#include <Vadon/Render/Canvas/Layer.hpp>
#include <Vadon/Render/Canvas/Item.hpp>
namespace Vadon::Private::Render::Canvas
{
	using LayerInfo = Vadon::Render::Canvas::LayerInfo;
	using LayerHandle = Vadon::Render::Canvas::LayerHandle;

	struct LayerData
	{
		LayerInfo info;
		std::vector<Vadon::Render::Canvas::ItemHandle> items;
		// TODO: anything else?
	};
}
#endif