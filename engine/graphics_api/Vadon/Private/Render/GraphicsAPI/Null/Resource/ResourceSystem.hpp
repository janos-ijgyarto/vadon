#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_RESOURCE_RESOURCESYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_NULL_RESOURCE_RESOURCESYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Resource/ResourceSystem.hpp>
namespace Vadon::Private::Render::Null
{
	class GraphicsAPI;

	class ResourceSystem final : public Vadon::Render::ResourceSystem
	{
	public:
		bool is_resource_valid(Vadon::Render::ResourceHandle /*resource_handle*/) const override { return false; }
		void remove_resource(Vadon::Render::ResourceHandle /*resource_handle*/) override {}

		Vadon::Render::SRVHandle create_srv(Vadon::Render::ResourceHandle /*resource_handle*/, const Vadon::Render::ShaderResourceViewInfo& /*srv_info*/) override { return Vadon::Render::SRVHandle{}; }
		Vadon::Render::ResourceHandle get_srv_resource(Vadon::Render::SRVHandle /*srv_handle*/) const override { return Vadon::Render::ResourceHandle{}; }
		void remove_srv(Vadon::Render::SRVHandle /*srv_handle*/) override {}

		Vadon::Render::UAVHandle create_uav(Vadon::Render::ResourceHandle /*resource_handle*/, const Vadon::Render::UnorderedAccessViewInfo& /*uav_info*/) override { return Vadon::Render::UAVHandle{}; }
		Vadon::Render::ResourceHandle get_uav_resource(Vadon::Render::UAVHandle /*uav_handle*/) const override { return Vadon::Render::ResourceHandle{}; }
		void remove_uav(Vadon::Render::UAVHandle /*uav_handle*/) override {}

		void update_subresource(Vadon::Render::ResourceHandle /*resource_handle*/, uint32_t /*subresource*/, const Vadon::Render::ResourceBox* /*box*/, const void* /*source_data*/, uint32_t /*source_row_pitch*/, uint32_t /*source_depth_pitch*/, Vadon::Render::CopyFlags /*flags*/) override {}

		bool map_resource(Vadon::Render::ResourceHandle /*resource_handle*/, uint32_t /*subresource*/, Vadon::Render::MapType /*map_type*/, Vadon::Render::MappedSubresource* /*mapped_subresource*/, Vadon::Render::MapFlags /*flags*/) override { return false; }
		void unmap_resource(Vadon::Render::ResourceHandle /*resource_handle*/, uint32_t /*subresource*/) override {}

		void apply_shader_resource(Vadon::Render::ShaderType /*shader_type*/, Vadon::Render::SRVHandle /*srv_handle*/, uint32_t /*slot*/) override {}
		void apply_shader_resource_slots(Vadon::Render::ShaderType /*shader_type*/, const Vadon::Render::ShaderResourceSpan& /*resource_views*/) override {}
	private:
		ResourceSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
			: Vadon::Render::ResourceSystem(core)
			, m_graphics_api(graphics_api)
		{ }

		GraphicsAPI& m_graphics_api;

		friend GraphicsAPI;
	};
}
#endif