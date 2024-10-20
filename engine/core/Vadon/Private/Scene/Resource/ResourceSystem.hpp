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
		Vadon::Utilities::TypeID get_resource_type_id(ResourceHandle resource_handle) const override;

		ResourcePath get_resource_path(ResourceHandle resource_handle) const override;
		void set_resource_path(ResourceHandle resource_handle, ResourcePath path) override;

		ResourceHandle find_resource(ResourceID resource_id) const override;
		bool load_resource(ResourceHandle resource_handle) override;

		std::vector<ResourceHandle> find_resources_of_type(Vadon::Utilities::TypeID type_id) const override;

		ResourceHandle find_resource(ResourcePath resource_path) const override;
		ResourceHandle import_resource(ResourcePath resource_path) override;
		bool export_resource(ResourceHandle resource_handle) override;

		bool serialize_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, ResourceHandle& property_value) override;
	protected:
		ResourceHandle internal_create_resource(ResourceBase* resource, Vadon::Utilities::TypeID type_id) override;
	private:
		class LoadContext;

		struct ResourceData
		{
			ResourcePath path;
			ResourceID id;
			Vadon::Utilities::TypeID type_id;
			ResourceBase* resource = nullptr;

			bool is_loaded() const { return resource != nullptr; }
		};

		ResourceSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		void shutdown();

		ResourceHandle create_resource_data(ResourceID resource_id, Vadon::Utilities::TypeID type_id);
		bool load_resource(Vadon::Scene::ResourceSystemInterface& context, ResourceHandle resource_handle);
		bool internal_load_resource(Vadon::Scene::ResourceSystemInterface& context, ResourceData& resource_data);

		bool serialize_resource_data(Vadon::Scene::ResourceSystemInterface& context, Vadon::Utilities::Serializer& serializer, const ResourceData& resource_data);

		using ResourcePool = Vadon::Utilities::ObjectPool<Vadon::Scene::Resource, ResourceData>;
		ResourcePool m_resource_pool;

		std::unordered_map<ResourceID, ResourceHandle> m_resource_lookup;

		friend class SceneSystem;
	};
}
#endif