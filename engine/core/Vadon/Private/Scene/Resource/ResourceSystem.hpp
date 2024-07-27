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
		const ResourceBase* get_resource_base(ResourceID resource_id) const override;

		ResourceID get_resource_id(ResourceHandle resource_handle) const override;

		ResourceHandle load_resource(ResourceID resource_id) override;
		ResourceHandle load_resource(std::string_view resource_path) override;
	protected:
		ResourceHandle internal_create_resource(ResourceBase* resource) override;
	private:
		struct ResourceData
		{
			ResourceID id;
			std::string path;
			ResourceBase* resource = nullptr;

			~ResourceData()
			{
				if (resource != nullptr)
				{
					delete resource;
					resource = nullptr;
				}
			}
		};

		ResourceSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		ResourceHandle internal_find_resource(ResourceID resource_id) const;
		ResourceHandle internal_create_resource(ResourceID resource_id, ResourceBase* resource);

		using ResourcePool = Vadon::Utilities::ObjectPool<Vadon::Scene::Resource, ResourceData>;
		ResourcePool m_resource_pool;

		std::unordered_map<ResourceID, ResourceHandle> m_resource_lookup;

		friend class SceneSystem;
	};
}
#endif