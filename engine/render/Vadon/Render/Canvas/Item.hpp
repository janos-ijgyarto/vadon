#ifndef VADON_RENDER_CANVAS_ITEM_HPP
#define VADON_RENDER_CANVAS_ITEM_HPP
#include <Vadon/Render/Canvas/Layer.hpp>
namespace Vadon::Render::Canvas
{
	struct ItemInfo
	{
		LayerHandle layer;
		Vector2 position = { 0, 0 };
		float scale = 1.0f;
		// TODO: proper 2D transform?
	};

	// FIXME: should this use a dense pool for faster iteration?
	// The commands will probably be more important
	VADON_DECLARE_TYPED_POOL_HANDLE(Item, ItemHandle);
}
#endif