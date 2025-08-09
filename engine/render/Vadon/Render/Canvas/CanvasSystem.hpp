#ifndef VADON_RENDER_CANVAS_CANVASSYSTEM_HPP
#define VADON_RENDER_CANVAS_CANVASSYSTEM_HPP
#include <Vadon/Render/RenderModule.hpp>

#include <Vadon/Render/Canvas/Batch.hpp>
#include <Vadon/Render/Canvas/Item.hpp>
#include <Vadon/Render/Canvas/Material.hpp>
#include <Vadon/Render/Canvas/Primitive.hpp>

namespace Vadon::Render::Canvas
{
	struct RenderContext;

	class CanvasSystem : public RenderSystem<CanvasSystem>
	{
	public:
		virtual LayerHandle create_layer(LayerInfo info) = 0;
		virtual bool is_layer_valid(LayerHandle layer_handle) const = 0;
		virtual LayerInfo get_layer_info(LayerHandle layer_handle) const = 0;
		virtual void remove_layer(LayerHandle layer_handle) = 0;

		virtual LayerHandle get_default_layer() const = 0;

		virtual void set_layer_transform(LayerHandle layer_handle, const Transform& transform) = 0;
		virtual void set_layer_flags(LayerHandle layer_handle, LayerFlags flags) = 0;

		virtual ItemHandle create_item(ItemInfo info) = 0;
		virtual bool is_item_valid(ItemHandle item_handle) const = 0;
		virtual ItemInfo get_item_info(ItemHandle item_handle) const = 0;
		virtual void remove_item(ItemHandle item_handle) = 0;

		virtual void set_item_layer(ItemHandle item_handle, LayerHandle layer_handle) = 0;
		virtual void set_item_visible(ItemHandle item_handle, bool visible) = 0;
		virtual void set_item_transform(ItemHandle item_handle, const Transform& transform) = 0;
		virtual void set_item_z_order(ItemHandle item_handle, float z_order) = 0;

		// Item commands
		virtual size_t get_item_buffer_size(ItemHandle item_handle) const = 0;
		virtual void clear_item(ItemHandle item_handle) = 0;
		virtual void add_item_batch_draw(ItemHandle item_handle, const BatchDrawCommand& batch_command) = 0;
		// FIXME: allow list versions to make this more efficient?
		virtual void draw_item_triangle(ItemHandle item_handle, const Triangle& triangle) = 0;
		virtual void draw_item_rectangle(ItemHandle item_handle, const Rectangle& rectangle) = 0;
		virtual void draw_item_circle(ItemHandle item_handle, const Circle& circle) = 0;
		virtual void draw_item_sprite(ItemHandle item_handle, const Sprite& sprite) = 0;

		// NOTE: this follows a D3D-style pattern of setting "render state" which applies to all subsequent operations
		// Might need further revision
		virtual void set_item_texture(ItemHandle item_handle, const Texture& texture) = 0;
		virtual void set_item_material(ItemHandle item_handle, MaterialHandle material_handle) = 0;

		virtual MaterialHandle create_material(MaterialInfo info) = 0;
		virtual bool is_material_valid(MaterialHandle material_handle) const = 0;
		virtual MaterialInfo get_material_info(MaterialHandle material_handle) const = 0;
		virtual void remove_material(MaterialHandle material_handle) = 0;

		virtual void set_material_sdf(MaterialHandle material_handle, SDFParameters parameters) = 0;

		virtual BatchHandle create_batch() = 0;
		virtual bool is_batch_valid(BatchHandle batch_handle) const = 0;
		virtual void remove_batch(BatchHandle batch_handle) = 0;
				
		// Batch commands
		virtual size_t get_batch_buffer_size(BatchHandle batch_handle) const = 0;
		virtual void clear_batch(BatchHandle batch_handle) = 0;
		// FIXME: allow list versions to make this more efficient?
		virtual void draw_batch_triangle(BatchHandle batch_handle, const Triangle& triangle) = 0;
		virtual void draw_batch_rectangle(BatchHandle batch_handle, const Rectangle& rectangle) = 0;
		virtual void draw_batch_circle(BatchHandle batch_handle, const Circle& circle) = 0;
		virtual void draw_batch_sprite(BatchHandle batch_handle, const Sprite& sprite) = 0;

		virtual void set_batch_texture(BatchHandle batch_handle, const Texture& texture) = 0;
		virtual void set_batch_material(BatchHandle batch_handle, MaterialHandle material_handle) = 0;

		// TODO: add separate function to just process a set of layers into a list of graphics commands
		// Can then draw them to a provided context (camera + viewports)

		virtual void render(const RenderContext& context) = 0;
	protected:
		CanvasSystem(Core::EngineCoreInterface& core)
			: System(core)
		{}
	};
}
#endif