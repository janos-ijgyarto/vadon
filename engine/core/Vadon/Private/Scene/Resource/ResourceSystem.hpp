#ifndef VADON_PRIVATE_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#define VADON_PRIVATE_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Private/Scene/Resource/Resource.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace Vadon::Private::Core
{
	class EngineCore;
}
namespace Vadon::Private::Scene
{
	class ResourceSystem final : public Vadon::Scene::ResourceSystem
	{
	public:
		ResourceHandle create_resource(Vadon::Utilities::TypeID type_id) override;
		ResourceHandle find_resource(ResourceID resource_id) const override;

		ResourceInfo get_resource_info(ResourceHandle resource_handle) const override;
		void set_resource_path(ResourceHandle resource_handle, const ResourcePath& path) override;

		std::vector<ResourceHandle> find_resources_of_type(Vadon::Utilities::TypeID type_id) const override;

		ResourceHandle import_resource(const ResourcePath& path) override;
		ResourceHandle load_resource(const ResourcePath& path) override;

		bool save_resource(ResourceHandle resource_handle) override;
		bool load_resource(ResourceHandle resource_handle) override;

		bool serialize_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle& resource_handle) override;
		bool serialize_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, ResourceHandle& property_value) override;

		void unload_resource(ResourceHandle resource_handle) override;
		void remove_resource(ResourceHandle resource_handle) override;

		bool is_resource_loaded(ResourceHandle resource_handle) const override;

		const ResourceBase* get_resource_base(ResourceHandle resource_handle) const override;
	private:
		struct ResourceData
		{
			ResourceInfo info;
			ResourceBase* resource = nullptr;

			bool is_loaded() const { return resource != nullptr; }
		};

		ResourceSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		void shutdown();

		bool internal_load_resource_info(Vadon::Utilities::Serializer& serializer, ResourceInfo& info) const;

		bool internal_save_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle resource_handle);
		bool internal_load_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle& resource_handle);
		ResourceBase* internal_load_resource_data(Vadon::Utilities::Serializer& serializer, const ResourceInfo& info);

		ResourceBase* internal_create_resource(Vadon::Utilities::TypeID type_id) const;
		ResourceHandle internal_add_resource(const ResourceInfo& info, ResourceBase* resource);

		using ResourcePool = Vadon::Utilities::ObjectPool<Vadon::Scene::Resource, ResourceData>;
		ResourcePool m_resource_pool;

		std::unordered_map<ResourceID, ResourceHandle> m_resource_lookup;

		friend Core::EngineCore;
	};
}
#endif