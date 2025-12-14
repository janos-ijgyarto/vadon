#ifndef VADON_PRIVATE_RENDER_CANVAS_CANVASSYSTEM_HPP
#define VADON_PRIVATE_RENDER_CANVAS_CANVASSYSTEM_HPP
#include <Vadon/Render/Canvas/CanvasSystem.hpp>

#include <Vadon/Private/Render/Canvas/Batch.hpp>
#include <Vadon/Private/Render/Canvas/Context.hpp>
#include <Vadon/Private/Render/Canvas/Item.hpp>
#include <Vadon/Private/Render/Canvas/Layer.hpp>
#include <Vadon/Private/Render/Canvas/Material.hpp>
#include <Vadon/Private/Render/Canvas/Primitive.hpp>
#include <Vadon/Private/Render/Canvas/Transform.hpp>

#include <Vadon/Render/GraphicsAPI/Buffer/Buffer.hpp>
#include <Vadon/Render/GraphicsAPI/Pipeline/Pipeline.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/Texture.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

namespace Vadon::Private::Render
{
	class RenderSystem;
}

namespace Vadon::Private::Render::Canvas
{
	class CanvasSystem : public Vadon::Render::Canvas::CanvasSystem
	{
	public:
		LayerHandle create_layer(LayerInfo info) override;
		bool is_layer_valid(LayerHandle layer_handle) const override { return m_layer_pool.is_handle_valid(layer_handle); }
		LayerInfo get_layer_info(LayerHandle layer_handle) const override;
		void remove_layer(LayerHandle layer_handle) override;

		LayerHandle get_default_layer() const override { return m_default_layer; }

		void set_layer_transform(LayerHandle layer_handle, const Transform& transform) override;
		void set_layer_flags(LayerHandle layer_handle, LayerFlags flags) override;

		ItemHandle create_item(ItemInfo info) override;
		bool is_item_valid(ItemHandle item_handle) const override { return m_item_pool.is_handle_valid(item_handle); }
		ItemInfo get_item_info(ItemHandle item_handle) const override;
		void remove_item(ItemHandle item_handle) override;

		void set_item_layer(ItemHandle item_handle, LayerHandle layer_handle) override;
		void set_item_visible(ItemHandle item_handle, bool visible) override;
		void set_item_transform(ItemHandle item_handle, const Transform& transform) override;
		void set_item_z_order(ItemHandle item_handle, float z_order) override;

		CommandBuffer& get_item_command_buffer(ItemHandle item_handle) override;
		MaterialOverride& get_item_material_override(ItemHandle item_handle) override;

		MaterialHandle create_material(MaterialInfo info) override;
		bool is_material_valid(MaterialHandle material_handle) const override { return m_material_pool.is_handle_valid(material_handle); }
		MaterialInfo get_material_info(MaterialHandle material_handle) const override;
		void remove_material(MaterialHandle material_handle) override;

		void set_material_sdf(MaterialHandle material_handle, SDFParameters parameters) override;

		BatchHandle create_batch() override;
		bool is_batch_valid(BatchHandle batch_handle) const override { return m_batch_pool.is_handle_valid(batch_handle); }
		void remove_batch(BatchHandle batch_handle) override;

		CommandBuffer& get_batch_command_buffer(BatchHandle batch_handle);

		void render(const RenderContext& context) override;
	protected:
		CanvasSystem(Vadon::Core::EngineCoreInterface& core)
			: Vadon::Render::Canvas::CanvasSystem(core)
		{}

		bool initialize();
		void shutdown();

		void init_render_state(const RenderContext& context);

		void update_shared_data();
		void update_frame_data(const RenderContext& context);

		void update_layers();
		void update_materials();
		void buffer_frame_data();

		void set_item_layer_dirty(const ItemData& item);
		void update_layer_items(LayerData& layer);

		LayerHandle get_layer_or_default(LayerHandle layer_handle) const;

		// Data shared across render passes (layers, materials, etc.)
		struct SharedData
		{
			// FIXME: merge updates if they are within a contiguous range?
			std::vector<LayerHandle> dirty_layers;
			std::vector<MaterialHandle> dirty_materials;

			void add_dirty_layer(LayerHandle layer) { dirty_layers.push_back(layer); }
			void add_dirty_material(MaterialHandle material) { dirty_materials.push_back(material); }

			void clear()
			{
				dirty_layers.clear();
				dirty_materials.clear();
			}
		};

		// Per-frame data (i.e update for every rendered pass)
		struct FrameData
		{
			struct PrimitiveBatch
			{
				int32_t index_count = 0;
				Vadon::Render::SRVHandle texture_srv;
				RenderState render_state;
			};

			std::vector<Vector4> primitive_data;
			std::vector<uint32_t> indices;

			// Replace with more generic API which can encapsulate both the draw command and other render state
			// (resource slots, etc.)
			// Allow introspection so compatible commands can be merged
			std::vector<PrimitiveBatch> primitive_batches;
			MaterialHandle current_material;
			Texture current_texture;
			RenderState current_render_state;

			bool is_valid() const { return primitive_batches.empty() == false; }

			void clear()
			{
				primitive_data.clear();
				indices.clear();
				current_material.invalidate();

				primitive_batches.clear();
			}

			void add_command(PrimitiveType primitive_type, Vadon::Render::SRVHandle texture_srv, const RenderState& render_state);
			void add_primitive_indices(PrimitiveType primitive_type);

			template<typename T>
			T* add_primitive_data(const T& data)
			{
				const size_t prev_size = primitive_data.size();
				const Vector4* data_ptr = (const Vector4*)&data;
				primitive_data.insert(primitive_data.end(), data_ptr, data_ptr + (sizeof(T) / sizeof(Vector4)));
				
				return reinterpret_cast<T*>(primitive_data.data() + prev_size);
			}
		};

		void reset_frame_state();

		Vadon::Utilities::ObjectPool<Vadon::Render::Canvas::Batch, BatchData> m_batch_pool;
		Vadon::Utilities::ObjectPool<Vadon::Render::Canvas::Item, ItemData> m_item_pool;
		Vadon::Utilities::ObjectPool<Vadon::Render::Canvas::Layer, LayerData> m_layer_pool;
		Vadon::Utilities::ObjectPool<Vadon::Render::Canvas::Material, MaterialData> m_material_pool;

		// FIXME: create array of these so we can process in parallel?
		SharedData m_shared_data;
		FrameData m_frame_data;

		// TODO: depth buffer & sorting via Z order?
		Vadon::Render::PipelineState m_pipeline_state;
		Vadon::Render::BlendStateHandle m_blend_disabled_state;
		Vadon::Render::BlendStateHandle m_alpha_blend_state;
		Vadon::Render::ShaderObject m_shader;

		Vadon::Render::BufferHandle m_material_buffer;
		Vadon::Render::SRVHandle m_material_buffer_srv;

		Vadon::Render::BufferInfo m_primitive_buffer_info;
		Vadon::Render::BufferHandle m_primitive_buffer;

		Vadon::Render::BufferSRVInfo m_primitive_buffer_srv_info;
		Vadon::Render::SRVHandle m_primitive_buffer_srv;

		Vadon::Render::BufferInfo m_index_buffer_info;
		Vadon::Render::BufferHandle m_index_buffer;

		Vadon::Render::BufferHandle m_view_cbuffer;
		Vadon::Render::BufferHandle m_layers_cbuffer;

		LayerHandle m_default_layer;
		MaterialHandle m_default_material;

		Vadon::Render::TextureHandle m_default_texture;
		Vadon::Render::SRVHandle m_default_texture_srv;
		Vadon::Render::TextureSamplerHandle m_sampler;

		friend Vadon::Private::Render::RenderSystem;
	};
}
#endif