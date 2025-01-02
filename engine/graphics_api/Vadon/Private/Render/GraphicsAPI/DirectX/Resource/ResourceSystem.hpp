#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RESOURCE_RESOURCESYSTEM_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RESOURCE_RESOURCESYSTEM_HPP
#include <Vadon/Render/GraphicsAPI/Resource/ResourceSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/Resource.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/SRV.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/UAV.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Shader.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace Vadon::Private::Render::DirectX
{
	class GraphicsAPI;

	using CopyFlags = Vadon::Render::CopyFlags;
	using MapType = Vadon::Render::MapType;
	using MapFlags = Vadon::Render::MapFlags;
	using MappedSubresource = Vadon::Render::MappedSubresource;
	using ResourceBox = Vadon::Render::ResourceBox;
	using ShaderResourceSpan = Vadon::Render::ShaderResourceSpan;

	class ResourceSystem final : public Vadon::Render::ResourceSystem
	{
	public:
		template<typename T> const T* get_resource(TypedResourceHandle<T> resource_handle) const
		{
			static_assert(std::is_base_of_v<Resource, T>, "Graphics resource system: type is not a subclass of Resource!");

			// TODO: validate whether it's the correct type?
			return static_cast<const T*>(get_base_resource(resource_handle));
		}

		template<typename T> T* get_resource(TypedResourceHandle<T> resource_handle) { return const_cast<T*>(std::as_const(*this).get_resource<T>(resource_handle)); }

		const Resource* get_base_resource(ResourceHandle resource_handle) const;
		Resource* get_base_resource(ResourceHandle resource_handle) { return const_cast<Resource*>(std::as_const(*this).get_base_resource(resource_handle)); }

		template<typename T>
		TypedResourceHandle<T> create_resource(ResourceType type, D3DResource d3d_resource)
		{
			static_assert(std::is_base_of_v<Resource, T>, "Type is not subclass of DirectX::Resource!");
			T* new_resource = new T();
			return TypedResourceHandle<T>::from_resource_handle(internal_create_resource(type, d3d_resource, new_resource));
		}

		bool is_resource_valid(ResourceHandle resource_handle) const override { return m_resource_pool.is_handle_valid(resource_handle); }
		void remove_resource(ResourceHandle resource_handle) override;

		SRVHandle create_srv(ResourceHandle resource_handle, const ShaderResourceViewInfo& srv_info) override;
		ResourceHandle get_srv_resource(SRVHandle srv_handle) const override;
		void remove_srv(SRVHandle srv_handle) override;

		UAVHandle create_uav(ResourceHandle resource_handle, const UnorderedAccessViewInfo& uav_info) override;
		ResourceHandle get_uav_resource(UAVHandle uav_handle) const override;
		void remove_uav(UAVHandle uav_handle) override;

		void update_subresource(ResourceHandle resource_handle, uint32_t subresource, const ResourceBox* box, const void* source_data, uint32_t source_row_pitch, uint32_t source_depth_pitch, CopyFlags flags) override;

		bool map_resource(ResourceHandle resource_handle, uint32_t subresource, MapType map_type, MappedSubresource* mapped_subresource, MapFlags flags) override;
		void unmap_resource(ResourceHandle resource_handle, uint32_t subresource) override;

		void apply_shader_resource(ShaderType shader_type, SRVHandle srv_handle, uint32_t slot) override;
		void apply_shader_resource_slots(ShaderType shader_type, const ShaderResourceSpan& resource_views) override;
	private:
		ResourceSystem(Vadon::Core::EngineCoreInterface& core, GraphicsAPI& graphics_api);

		bool initialize();
		void shutdown();

		ResourceHandle internal_create_resource(ResourceType type, D3DResource d3d_resource, Resource* resource);

		struct ResourceWrapper
		{
			Resource* resource = nullptr;
		};

		GraphicsAPI& m_graphics_api;

		Vadon::Utilities::ObjectPool<Vadon::Render::Resource, ResourceWrapper> m_resource_pool;
		Vadon::Utilities::ObjectPool<Vadon::Render::ShaderResourceView, ShaderResourceView> m_srv_pool;
		Vadon::Utilities::ObjectPool<Vadon::Render::UnorderedAccessView, UnorderedAccessView> m_uav_pool;

		friend GraphicsAPI;
	};
}
#endif