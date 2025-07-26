#ifndef VADONDEMO_RENDER_RENDER_HPP
#define VADONDEMO_RENDER_RENDER_HPP
#include <VadonDemo/VadonDemoCommon.hpp>

#include <VadonDemo/Render/Component.hpp>

#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Render/Canvas/Context.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

#include <unordered_map>

namespace Vadon::Core
{
	struct FileSystemPath;
}
namespace Vadon::ECS
{
	class World;
}
namespace Vadon::Render::Canvas
{
	class CanvasSystem;
	struct Camera;
}
namespace VadonDemo::Core
{
	class Core;
}
namespace VadonDemo::Render
{
	class Render
	{
	public:
		static void register_types();

		// NOTE: each new Context will create its own Layer instances
		VADONDEMO_API CanvasContextHandle create_canvas_context();
		VADONDEMO_API Vadon::Render::Canvas::RenderContext& get_context(CanvasContextHandle context_handle);

		VADONDEMO_API void init_canvas_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity, CanvasContextHandle context_handle);
		VADONDEMO_API void update_canvas_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
		VADONDEMO_API void remove_canvas_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);

		VADONDEMO_API void update_layer_definition(CanvasLayerDefHandle layer_def_handle);

		VADONDEMO_API void init_sprite_tiling_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
		VADONDEMO_API void update_sprite_tiling_entity(const CanvasComponent& canvas_component, SpriteTilingComponent& sprite_component, const Vadon::Render::Rectangle& culling_rect);
		VADONDEMO_API void remove_sprite_tiling_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);

		VADONDEMO_API void init_fullscreen_effect_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
		VADONDEMO_API void update_fullscreen_effect_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
		VADONDEMO_API void remove_fullscreen_effect_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);

		VADONDEMO_API bool init_texture_resource(TextureResourceID texture_id) const;
		VADONDEMO_API void unload_texture_resource(TextureResourceID texture_id) const;

		VADONDEMO_API bool init_shader_resource(ShaderResourceID shader_id) const;
		VADONDEMO_API void unload_shader_resource(ShaderResourceID shader_id) const;
	private:
		struct CanvasContextData
		{
			Vadon::Render::Canvas::RenderContext render_context;
			std::unordered_map<uint64_t, CanvasLayerDefHandle> layer_definitions;
		};

		Render(VadonDemo::Core::Core& core);

		bool initialize();
		void global_config_updated();

		Vadon::Render::Canvas::LayerHandle get_context_layer(CanvasContextHandle context_handle, CanvasLayerDefID layer_def_id);
		
		void sort_context_layers(CanvasContextData& context);

		VadonDemo::Core::Core& m_core;

		Vadon::Utilities::ObjectPool<CanvasContext, CanvasContextData> m_context_pool;

		friend Core::Core;
	};
}
#endif