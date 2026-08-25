#ifndef VADONDEMO_RENDER_COMPONENT_HPP
#define VADONDEMO_RENDER_COMPONENT_HPP
#include <Vadon/Render/Canvas/Item.hpp>
#include <Vadon/Render/Utilities/Rectangle.hpp>
#include <VadonDemo/Render/Resource.hpp>
namespace Vadon::Foundation
{
	class TypeMetadataRegistry;
}
namespace VadonDemo::Render
{
	VADON_DECLARE_TYPED_POOL_HANDLE(CanvasContext, CanvasContextHandle);

	struct CanvasComponent
	{
		VADON_DECLARE_MEMBER_UUID(visible, "fb7713f6-8920-48a0-811d-8b1845788de1");
		VADON_DECLARE_MEMBER_UUID(z_order, "d41f0003-a8f6-4c7b-8da3-82f2bfbbc2a0");
		VADON_DECLARE_MEMBER_UUID(layer_def, "0006a253-4cf3-447a-8f73-2443d05c7873");

		bool visible = true;
		float z_order = 0.0f;

		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		CanvasLayerDefID layer_def;

		Vadon::Render::Canvas::ItemHandle canvas_item;
		CanvasContextHandle context_handle;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	// NOTE: this allows implementing "background tiling" based on the view position
	// An easier way to implement this could be just a view-agnostic sprite with a seamless texture where we modify the UVs
	struct SpriteTilingComponent
	{
		VADON_DECLARE_MEMBER_UUID(texture, "8f4d2c5b-a6fa-4ff3-997e-87fd3d5189a8");
		VADON_DECLARE_MEMBER_UUID(tile_size, "a0925183-440e-4629-b096-6e71e38e7124");
		VADON_DECLARE_MEMBER_UUID(repeat, "089b72e6-c19c-4d9b-a2f6-c4782248edcf");
		VADON_DECLARE_MEMBER_UUID(rotate, "6ffea325-c554-4a16-b83e-cc64d9a771c2");

		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		TextureResourceID texture;
		Vadon::Render::Vector2 tile_size = Vadon::Render::Vector2_One;
		bool repeat = false;
		bool rotate = false;

		Vadon::Render::RectangleInt tile_rect;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);

		void reset_rect()
		{
			tile_rect.position = { 0, 0 };
			tile_rect.size = { 0, 0 };
		}
	};

	struct FullscreenEffectComponent
	{
		VADON_DECLARE_MEMBER_UUID(shader, "c7ce9d3a-5836-41a5-9e28-5e71cd7892c4");
		VADON_DECLARE_MEMBER_UUID(view_agnostic, "d9426412-5cd1-4616-a2fa-2a5199783113");

		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		ShaderResourceID shader;
		bool view_agnostic = false;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};
}

VADON_REGISTER_TYPE_UUID(VadonDemo::Render::CanvasComponent, "7af45a53-1d4f-4430-aafd-98c32ff08da2");
VADON_REGISTER_TYPE_UUID(VadonDemo::Render::SpriteTilingComponent, "3c419918-e7ce-405d-aaac-d15c349fa4c7");
VADON_REGISTER_TYPE_UUID(VadonDemo::Render::FullscreenEffectComponent, "8a515350-c79d-432e-b1f6-993e743f8cf9");
#endif