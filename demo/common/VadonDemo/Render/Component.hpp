#ifndef VADONDEMO_RENDER_COMPONENT_HPP
#define VADONDEMO_RENDER_COMPONENT_HPP
#include <Vadon/Render/Canvas/Item.hpp>
#include <Vadon/Render/Utilities/Rectangle.hpp>
#include <VadonDemo/Render/Resource.hpp>
namespace VadonDemo::Render
{
	VADON_DECLARE_TYPED_POOL_HANDLE(CanvasContext, CanvasContextHandle);

	struct CanvasComponent
	{
		bool visible = true;
		float z_order = 0.0f;

		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		CanvasLayerDefID layer_def;

		Vadon::Render::Canvas::ItemHandle canvas_item;
		CanvasContextHandle context_handle;

		static void register_component();
	};

	// NOTE: this allows implementing "background tiling" based on the view position
	// An easier way to implement this could be just a view-agnostic sprite with a seamless texture where we modify the UVs
	struct SpriteTilingComponent
	{
		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		TextureResourceID texture;
		Vadon::Render::Vector2 tile_size = Vadon::Render::Vector2_One;
		bool repeat = false;
		bool rotate = false;

		Vadon::Render::RectangleInt tile_rect;

		static void register_component();

		void reset_rect()
		{
			tile_rect.position = { 0, 0 };
			tile_rect.size = { 0, 0 };
		}
	};

	struct FullscreenEffectComponent
	{
		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		ShaderResourceID shader;
		bool view_agnostic = false;

		static void register_component();
	};
}
#endif