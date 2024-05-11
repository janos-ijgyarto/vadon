#ifndef VADON_PRIVATE_RENDER_CANVAS_CANVASSYSTEM_HPP
#define VADON_PRIVATE_RENDER_CANVAS_CANVASSYSTEM_HPP
#include <Vadon/Render/Canvas/CanvasSystem.hpp>

#include <Vadon/Private/Render/Canvas/Item.hpp>
#include <Vadon/Private/Render/Canvas/Layer.hpp>
#include <Vadon/Private/Render/Canvas/Material.hpp>
#include <Vadon/Private/Render/Canvas/Primitive.hpp>

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
	using Camera = Vadon::Render::Canvas::Camera;
	using Viewport = Vadon::Render::Canvas::Viewport;
	using RenderContext = Vadon::Render::Canvas::RenderContext;

	class CanvasSystem : public Vadon::Render::Canvas::CanvasSystem
	{
	public:
		LayerHandle create_layer(LayerInfo info) override;
		bool is_layer_valid(LayerHandle layer_handle) const override { return m_layer_pool.is_handle_valid(layer_handle); }
		LayerInfo get_layer_info(LayerHandle layer_handle) const override;
		void remove_layer(LayerHandle layer_handle) override;

		void set_layer_offset(LayerHandle layer_handle, Vector2 offset) override;

		ItemHandle create_item(ItemInfo info) override;
		bool is_item_valid(ItemHandle item_handle) const override { return m_item_pool.is_handle_valid(item_handle); }
		ItemInfo get_item_info(ItemHandle item_handle) const override;
		void remove_item(ItemHandle item_handle) override;

		void clear_item(ItemHandle item_handle) override;
		void set_item_position(ItemHandle item_handle, Vector2 position) override;

		MaterialHandle create_material(MaterialInfo info) override;
		bool is_material_valid(MaterialHandle material_handle) const override { return m_material_pool.is_handle_valid(material_handle); }
		MaterialInfo get_material_info(MaterialHandle material_handle) const override;
		void remove_material(MaterialHandle material_handle) override;

		void set_material_sdf(MaterialHandle material_handle, SDFParameters parameters) override;

		void draw_triangle(ItemHandle item_handle, const Triangle& triangle) override;
		void draw_rectangle(ItemHandle item_handle, const Rectangle& rectangle) override;
		void draw_sprite(ItemHandle item_handle, const Sprite& sprite) override;

		void render(const RenderContext& context) override;
	protected:
		CanvasSystem(Vadon::Core::EngineCoreInterface& core)
			: Vadon::Render::Canvas::CanvasSystem(core)
		{}

		bool initialize();

		void init_render_state(const RenderContext& context);

		void update_shared_data();
		void update_frame_data(const RenderContext& context);

		void update_layers();
		void update_materials();
		void buffer_frame_data();

		uint32_t get_material_index(const PrimitiveBase& primitive);

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
				Vadon::Render::ResourceViewHandle texture;
			};

			std::vector<Vector4> primitive_data;
			std::vector<uint32_t> indices;

			std::vector<PrimitiveBatch> primitive_batches;

			bool is_valid() const { return primitive_batches.empty() == false; }

			void clear()
			{
				primitive_data.clear();
				indices.clear();

				primitive_batches.clear();
			}

			void add_command(PrimitiveType primitive_type, Vadon::Render::ResourceViewHandle texture);
			void add_primitive_indices(PrimitiveType primitive_type);

			template<typename T>
			void add_primitive_data(const T& data)
			{
				const Vector4* data_ptr = (const Vector4*)&data;
				primitive_data.insert(primitive_data.end(), data_ptr, data_ptr + (sizeof(T) / sizeof(Vector4)));
			}
		};

		Vadon::Utilities::ObjectPool<Vadon::Render::Canvas::Item, ItemData> m_item_pool;
		Vadon::Utilities::ObjectPool<Vadon::Render::Canvas::Layer, LayerData> m_layer_pool;
		Vadon::Utilities::ObjectPool<Vadon::Render::Canvas::Material, MaterialData> m_material_pool;

		SharedData m_shared_data;
		FrameData m_frame_data;

		// TODO: depth buffer & sorting via Z order?
		Vadon::Render::PipelineState m_pipeline_state;
		Vadon::Render::BlendStateHandle m_blend_disabled_state;
		Vadon::Render::BlendStateHandle m_alpha_blend_state;
		Vadon::Render::ShaderObject m_shader;

		Vadon::Render::BufferObject m_material_buffer;
		Vadon::Render::BufferObject m_primitive_buffer;
		Vadon::Render::BufferObject m_index_buffer;

		Vadon::Render::BufferObject m_view_cbuffer;
		Vadon::Render::BufferObject m_layers_cbuffer;

		MaterialHandle m_default_material;

		Vadon::Render::TextureObject m_default_texture;
		Vadon::Render::TextureSamplerHandle m_sampler;

		friend Vadon::Private::Render::RenderSystem;
	};
}
#endif