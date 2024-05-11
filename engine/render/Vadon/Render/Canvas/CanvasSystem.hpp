#ifndef VADON_RENDER_CANVAS_CANVASSYSTEM_HPP
#define VADON_RENDER_CANVAS_CANVASSYSTEM_HPP
#include <Vadon/Render/RenderModule.hpp>

#include <Vadon/Render/Canvas/Item.hpp>
#include <Vadon/Render/Canvas/Layer.hpp>
#include <Vadon/Render/Canvas/Material.hpp>
#include <Vadon/Render/Canvas/Primitive.hpp>

#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>

namespace Vadon::Render::Canvas
{
	struct Camera
	{
		Vadon::Render::Rectangle view_rectangle;
		float zoom = 1.0f;
	};

	struct Viewport
	{
		Vadon::Render::RenderTargetHandle render_target;
		Vadon::Render::Viewport render_viewport;
	};

	struct RenderContext
	{
		Camera camera;
		std::vector<LayerHandle> layers;
		std::vector<Viewport> viewports;
	};

	class CanvasSystem : public RenderSystem<CanvasSystem>
	{
	public:
		virtual LayerHandle create_layer(LayerInfo info) = 0;
		virtual bool is_layer_valid(LayerHandle layer_handle) const = 0;
		virtual LayerInfo get_layer_info(LayerHandle layer_handle) const = 0;
		virtual void remove_layer(LayerHandle layer_handle) = 0;

		virtual void set_layer_offset(LayerHandle layer_handle, Vector2 offset) = 0;

		virtual ItemHandle create_item(ItemInfo info) = 0;
		virtual bool is_item_valid(ItemHandle item_handle) const = 0;
		virtual ItemInfo get_item_info(ItemHandle item_handle) const = 0;
		virtual void remove_item(ItemHandle item_handle) = 0;

		virtual void clear_item(ItemHandle item_handle) = 0;
		virtual void set_item_position(ItemHandle item_handle, Vector2 position) = 0;

		virtual MaterialHandle create_material(MaterialInfo info) = 0;
		virtual bool is_material_valid(MaterialHandle material_handle) const = 0;
		virtual MaterialInfo get_material_info(MaterialHandle material_handle) const = 0;
		virtual void remove_material(MaterialHandle material_handle) = 0;

		virtual void set_material_sdf(MaterialHandle material_handle, SDFParameters parameters) = 0;

		// FIXME: allow list versions to make this more efficient?
		virtual void draw_triangle(ItemHandle item_handle, const Triangle& triangle) = 0;
		virtual void draw_rectangle(ItemHandle item_handle, const Rectangle& rectangle) = 0;
		virtual void draw_sprite(ItemHandle item_handle, const Sprite& sprite) = 0;

		virtual void render(const RenderContext& context) = 0;
	protected:
		CanvasSystem(Core::EngineCoreInterface& core)
			: System(core)
		{}
	};
}
#endif