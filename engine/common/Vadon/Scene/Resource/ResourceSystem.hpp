#ifndef VADON_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#define VADON_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#include <Vadon/Scene/Module.hpp>
#include <Vadon/Scene/Resource/Registry.hpp>
namespace Vadon::Scene
{
	class ResourceSystemInterface
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
			return internal_create_resource(ResourceRegistry::create_resource(type_id), type_id);
		}

		template<typename T>
		const T* get_resource(ResourceHandle resource_handle) const
		{
			static_assert(std::is_base_of_v<ResourceBase, T>);
			return static_cast<const T*>(get_resource_base(resource_handle));
		}

		template<typename T>
		T* get_resource(ResourceHandle resource_handle) { return const_cast<T*>(std::as_const(*this).get_resource<T>(resource_handle)); }

		virtual const ResourceBase* get_resource_base(ResourceHandle resource_handle) const = 0;
		ResourceBase* get_resource_base(ResourceHandle resource_handle) { return const_cast<ResourceBase*>(std::as_const(*this).get_resource_base(resource_handle)); }

		virtual ResourceID get_resource_id(ResourceHandle resource_handle) const = 0;
		virtual Vadon::Utilities::TypeID get_resource_type_id(ResourceHandle resource_handle) const = 0;

		virtual ResourcePath get_resource_path(ResourceHandle resource_handle) const = 0;
		virtual void set_resource_path(ResourceHandle resource_handle, ResourcePath path) = 0;

		virtual ResourceHandle find_resource(ResourceID resource_id) const = 0;
		virtual bool load_resource(ResourceHandle resource_handle) = 0;

		template<typename T>
		std::vector<ResourceHandle> find_resources_of_type() const
		{
			static_assert(std::is_base_of_v<ResourceBase, T>);
			return find_resources_of_type(Vadon::Utilities::TypeRegistry::get_type_id<T>());
		}

		virtual std::vector<ResourceHandle> find_resources_of_type(Vadon::Utilities::TypeID type_id) const = 0;

		// FIXME: we should not be loading the resource yet, merely registering the path
		// Need some way to associate paths with IDs and store that metadata somehow
		virtual ResourceHandle find_resource(ResourcePath resource_path) const = 0;
		virtual ResourceHandle import_resource(ResourcePath resource_path) = 0;
		virtual bool export_resource(ResourceHandle resource_handle) = 0;

		// NOTE: this expecting to serialize between a resource handle property and resource ID data
		virtual bool serialize_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, ResourceHandle& property_value) = 0;
	protected:
		virtual ResourceHandle internal_create_resource(ResourceBase* resource, Vadon::Utilities::TypeID type_id) = 0;
	};

	class ResourceSystem : public SceneSystemBase<ResourceSystem>, public ResourceSystemInterface
	{
	protected:
		ResourceSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif