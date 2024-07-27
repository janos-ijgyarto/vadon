#ifndef VADON_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#define VADON_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#include <Vadon/Scene/Module.hpp>
#include <Vadon/Scene/Resource/Registry.hpp>
namespace Vadon::Scene
{
	class ResourceSystem : public SceneSystemBase<ResourceSystem>
	{
	public:
		template<typename T>
		ResourceHandle create_resource()
		{
			static_assert(std::is_base_of_v<ResourceBase, T>);
			return create_resource(Vadon::Utilities::TypeRegistry::get_type_id<T>());
		}

		ResourceHandle create_resource(Vadon::Utilities::TypeID type_id)
		{
			return internal_create_resource(ResourceRegistry::create_resource(type_id));
		}

		virtual ResourceHandle load_resource(ResourceID resource_id) = 0;

		// FIXME: we should not be loading the resource yet, merely registering the path
		// Need some way to associate paths with IDs and store that metadata somehow
		virtual ResourceHandle load_resource(std::string_view resource_path) = 0;

		template<typename T>
		const T* get_resource(ResourceID resource_id) const
		{
			static_assert(std::is_base_of_v<ResourceBase, T>);
			return static_cast<const T*>(get_resource_base(resource_id));
		}

		template<typename T>
		T* get_resource(ResourceID resource_id) { return const_cast<T*>(std::as_const(*this).get_resource<T>(resource_id)); }

		virtual const ResourceBase* get_resource_base(ResourceID resource_id) const = 0;
		ResourceBase* get_resource_base(ResourceID resource_id) { return const_cast<ResourceBase*>(std::as_const(*this).get_resource_base(resource_id)); }

		virtual ResourceID get_resource_id(ResourceHandle resource_handle) const = 0;
	protected:
		virtual ResourceHandle internal_create_resource(ResourceBase* resource) = 0;

		ResourceSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif