#ifndef VADON_PRIVATE_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#define VADON_PRIVATE_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Private/Scene/Resource/Resource.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace Vadon::Private::Scene
{
	class ResourceSystem final : public Vadon::Scene::ResourceSystem
	{
	public:
		const ResourceBase* get_resource_base(ResourceHandle resource_handle) const override;
		ResourceID get_resource_id(ResourceHandle resource_handle) const override;

		std::string get_resource_path(ResourceID resource_id) const override;
		void set_resource_path(ResourceID resource_id, std::string_view path) override;

		ResourceHandle find_resource(ResourceID resource_id) const override;
		ResourceHandle load_resource(ResourceID resource_id) override;

		std::vector<ResourceID> find_resources_of_type(Vadon::Utilities::TypeID type_id) const override;

		ResourceID find_resource(std::string_view resource_path) const override;
		ResourceID import_resource(std::string_view resource_path) override;
		bool export_resource(ResourceID resource_id) override;

		bool serialize_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value) override;
	protected:
		ResourceHandle internal_create_resource(ResourceBase* resource, Vadon::Utilities::TypeID type_id) override;
	private:
		class LoadContext;

		struct ResourceData
		{
			ResourceID id;
			ResourceBase* resource = nullptr;
		};

		struct ResourceInfo
		{
			Vadon::Utilities::TypeID type_id;
			std::string path;
			ResourceHandle handle;
		};

		ResourceSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		void shutdown();

		const ResourceInfo* internal_find_resource(ResourceID resource_id) const;
		ResourceInfo* internal_find_resource(ResourceID resource_id) { return const_cast<ResourceInfo*>(std::as_const(*this).internal_find_resource(resource_id)); }
		ResourceHandle internal_create_resource(ResourceID resource_id, ResourceInfo& info, ResourceBase* resource);

		ResourceHandle load_resource(Vadon::Scene::ResourceSystemInterface& context, ResourceID resource_id);
		ResourceHandle internal_load_resource(Vadon::Scene::ResourceSystemInterface& context, ResourceID resource_id, ResourceInfo& info);

		bool serialize_resource_data(Vadon::Scene::ResourceSystemInterface& context, Vadon::Utilities::Serializer& serializer, ResourceBase& resource, const ResourceInfo& info);

		using ResourcePool = Vadon::Utilities::ObjectPool<Vadon::Scene::Resource, ResourceData>;
		ResourcePool m_resource_pool;

		std::unordered_map<ResourceID, ResourceInfo> m_resource_lookup;

		friend class SceneSystem;
	};
}
#endif