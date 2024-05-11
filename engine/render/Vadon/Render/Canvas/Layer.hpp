#ifndef VADON_RENDER_CANVAS_LAYER_HPP
#define VADON_RENDER_CANVAS_LAYER_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Render/Utilities/Vector.hpp>
namespace Vadon::Render::Canvas
{
	struct LayerInfo
	{
		enum class Flags
		{
			NONE = 0,
			VIEW_AGNOSTIC = 1 << 0 // Offset and scale is relative to view position
		};

		Vector2 offset = { 0, 0 }; // Applied to all of the layer's contents during rendering (independent from transform hierarchy)
		Flags flags = Flags::NONE;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Layer, LayerHandle);
}
#endif