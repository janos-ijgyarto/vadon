#ifndef VADON_RENDER_CANVAS_LAYER_HPP
#define VADON_RENDER_CANVAS_LAYER_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Render/Canvas/Transform.hpp>
namespace Vadon::Render::Canvas
{
	enum class LayerFlags
	{
		NONE = 0,
		VIEW_AGNOSTIC = 1 << 0 // Offset and scale is relative to view position
	};

	struct LayerInfo
	{
		Transform transform; // Applied to all of the layer's contents during rendering (independent from transform hierarchy)
		LayerFlags flags = LayerFlags::NONE;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Layer, LayerHandle);
}
#endif