#ifndef VADON_RENDER_CANVAS_CANVASSYSTEM_HPP
#define VADON_RENDER_CANVAS_CANVASSYSTEM_HPP
#include <Vadon/Render/RenderModule.hpp>

#include <Vadon/Render/Canvas/Batch.hpp>
#include <Vadon/Render/Canvas/Item.hpp>
#include <Vadon/Render/Canvas/Material.hpp>
#include <Vadon/Render/Canvas/Primitive.hpp>

namespace Vadon::Render::Canvas
{
	class CommandBuffer;
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

		virtual CommandBuffer& get_item_command_buffer(ItemHandle item_handle) = 0;
		virtual MaterialOverride& get_item_material_override(ItemHandle item_handle) = 0;

		virtual MaterialHandle create_material(MaterialInfo info) = 0;
		virtual bool is_material_valid(MaterialHandle material_handle) const = 0;
		virtual MaterialInfo get_material_info(MaterialHandle material_handle) const = 0;
		virtual void remove_material(MaterialHandle material_handle) = 0;

		virtual void set_material_sdf(MaterialHandle material_handle, SDFParameters parameters) = 0;

		virtual BatchHandle create_batch() = 0;
		virtual bool is_batch_valid(BatchHandle batch_handle) const = 0;
		virtual void remove_batch(BatchHandle batch_handle) = 0;
		
		virtual CommandBuffer& get_batch_command_buffer(BatchHandle batch_handle) = 0;

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