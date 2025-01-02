#ifndef VADON_RENDER_GRAPHICSAPI_RESOURCE_RESOURCESYSTEM_HPP
#define VADON_RENDER_GRAPHICSAPI_RESOURCE_RESOURCESYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/Resource.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/SRV.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/UAV.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <span>
namespace Vadon::Render
{
	enum class CopyFlags
	{
		NONE = 0,
		NO_OVERWRITE = 1 << 0,
		DISCARD = 1 << 1
	};

	enum class MapType
	{
		READ,
		WRITE,
		READ_WRITE,
		WRITE_DISCARD,
		WRITE_NO_OVERWRITE
	};

	enum class MapFlags
	{
		NONE = 0,
		DO_NOT_WAIT = 1 << 0
	};

	struct MappedSubresource
	{
		void* data = nullptr;
		uint32_t row_pitch = 0;
		uint32_t depth_pitch = 0;
	};

	struct ResourceBox
	{
		uint32_t left;
		uint32_t top;
		uint32_t front;
		uint32_t right;
		uint32_t bottom;
		uint32_t back;
	};

	struct ShaderResourceSpan
	{
		int32_t start_slot = 0;
		std::span<SRVHandle> resources;
	};

	class ResourceSystem : public GraphicsSystem<ResourceSystem>
	{
	public:
		virtual bool is_resource_valid(ResourceHandle resource_handle) const = 0;

		// NOTE: this will only de-reference the resource itself
		// Views may still retain references in the backend
		virtual void remove_resource(ResourceHandle resource_handle) = 0;

		virtual SRVHandle create_srv(ResourceHandle resource_handle, const ShaderResourceViewInfo& srv_info) = 0;
		virtual ResourceHandle get_srv_resource(SRVHandle srv_handle) const = 0;
		virtual void remove_srv(SRVHandle srv_handle) = 0;

		virtual UAVHandle create_uav(ResourceHandle resource_handle, const UnorderedAccessViewInfo& uav_info) = 0;
		virtual ResourceHandle get_uav_resource(UAVHandle uav_handle) const = 0;
		virtual void remove_uav(UAVHandle uav_handle) = 0;

		virtual void update_subresource(ResourceHandle resource_handle, uint32_t subresource, const ResourceBox* box, const void* source_data, uint32_t source_row_pitch, uint32_t source_depth_pitch, CopyFlags flags = CopyFlags::NONE) = 0;

		virtual bool map_resource(ResourceHandle resource_handle, uint32_t subresource, MapType map_type, MappedSubresource* mapped_subresource, MapFlags flags = MapFlags::NONE) = 0;
		virtual void unmap_resource(ResourceHandle resource_handle, uint32_t subresource) = 0;

		// TODO: copy resource

		virtual void apply_shader_resource(ShaderType shader_type, SRVHandle srv_handle, uint32_t slot) = 0;
		virtual void apply_shader_resource_slots(ShaderType shader_type, const ShaderResourceSpan& resource_views) = 0;
	protected:
		ResourceSystem(Vadon::Core::EngineCoreInterface& engine_core)
			: System(engine_core)
		{
		}
	};
}
#endif