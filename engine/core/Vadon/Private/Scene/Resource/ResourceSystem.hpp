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
	using ResourceDatabase = Vadon::Scene::ResourceDatabase;

	class ResourceSystem final : public Vadon::Scene::ResourceSystem
	{
	public:
		ResourceHandle create_resource(Vadon::Utilities::TypeID type_id) override;
		ResourceHandle find_resource(ResourceID resource_id) const override;

		ResourceInfo get_resource_info(ResourceHandle resource_handle) const override;

		std::vector<ResourceHandle> find_resources_of_type(Vadon::Utilities::TypeID type_id) const override;

		void register_database(ResourceDatabase& database) override;

		bool load_resource_info(Vadon::Utilities::Serializer& serializer, ResourceInfo& resource_info) const override;

		bool save_resource(ResourceHandle resource_handle) override;
		ResourceHandle load_resource_base(ResourceID resource_id) override;

		bool save_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle resource_handle) override;
		ResourceHandle load_resource(Vadon::Utilities::Serializer& serializer) override;

		void remove_resource(ResourceHandle resource_handle) override;

		const Resource* get_base_resource(ResourceHandle resource_handle) const override;
	private:
		struct ResourceData
		{
			ResourceInfo info;
			Resource* resource = nullptr;
		};

		ResourceSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		void shutdown();

		Resource* load_resource_data(Vadon::Utilities::Serializer& serializer, const ResourceInfo& info);

		Resource* internal_create_resource(Vadon::Utilities::TypeID type_id) const;
		ResourceHandle internal_add_resource(const ResourceInfo& info, Resource* resource);

		using ResourcePool = Vadon::Utilities::ObjectPool<Vadon::Scene::ResourceBase, ResourceData>;
		ResourcePool m_resource_pool;

		std::unordered_map<ResourceID, ResourceHandle> m_resource_lookup;
		std::vector<ResourceDatabase*> m_database_list;

		friend Core::EngineCore;
	};
}
#endif