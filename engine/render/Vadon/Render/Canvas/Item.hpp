#ifndef VADON_RENDER_CANVAS_ITEM_HPP
#define VADON_RENDER_CANVAS_ITEM_HPP
#include <Vadon/Render/Canvas/Layer.hpp>
namespace Vadon::Render::Canvas
{
	struct ItemInfo
	{
		LayerHandle layer;
		Transform transform;
		float z_order = 0.0f;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Item, ItemHandle);
}
#endif