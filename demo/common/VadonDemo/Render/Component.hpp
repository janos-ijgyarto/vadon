#ifndef VADONDEMO_RENDER_COMPONENT_HPP
#define VADONDEMO_RENDER_COMPONENT_HPP
#include <Vadon/Render/Canvas/Item.hpp>
#include <VadonDemo/Render/Resource.hpp>
namespace VadonDemo::Render
{
	VADON_DECLARE_TYPED_POOL_HANDLE(CanvasContext, CanvasContextHandle);

	struct CanvasComponent
	{
		bool visible = true;
		float z_order = 0.0f;
		CanvasLayerDefHandle layer_def;

		Vadon::Render::Canvas::ItemHandle canvas_item;
		CanvasContextHandle context_handle;

		static void register_component();
	};

	struct FullscreenEffectComponent
	{
		FullscreenEffectHandle effect;

		// NOTE: For now we just implicitly draw FS effect first, then canvas
		// Need some form of "render config" later

		static void register_component();
	};
}
#endif