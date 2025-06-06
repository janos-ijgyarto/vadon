#ifndef VADON_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#define VADON_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#include <Vadon/Scene/Module.hpp>
#include <Vadon/Scene/Resource/Resource.hpp>
#include <Vadon/Utilities/TypeInfo/Registry.hpp>
namespace Vadon::Utilities
{
	class Serializer;
}
namespace Vadon::Scene
{
	class ResourceDatabase;

	class ResourceSystem : public SceneSystemBase<ResourceSystem>
	{
	public:
		template<typename T>
		ResourceHandle create_resource()
		{
			static_assert(std::is_base_of_v<Resource, T>);
			return create_resource(Vadon::Utilities::TypeRegistry::get_type_id<T>());
		}

		virtual ResourceHandle create_resource(Vadon::Utilities::TypeID type_id) = 0;

		virtual ResourceInfo get_resource_info(ResourceHandle resource_handle) const = 0;

		virtual ResourceHandle find_resource(ResourceID resource_id) const = 0;

		template<typename T>
		std::vector<ResourceHandle> find_resources_of_type() const
		{
			static_assert(std::is_base_of_v<Resource, T>);
			return find_resources_of_type(Vadon::Utilities::TypeRegistry::get_type_id<T>());
		}

		virtual std::vector<ResourceHandle> find_resources_of_type(Vadon::Utilities::TypeID type_id) const = 0;

		virtual void register_database(ResourceDatabase& database) = 0;

		virtual bool load_resource_info(Vadon::Utilities::Serializer& serializer, ResourceInfo& resource_info) const = 0;

		// This will query all the registered ResourceSerializers for this ID and attempt to serialize the resource through them
		virtual bool save_resource(ResourceHandle resource_handle) = 0;
		virtual ResourceHandle load_resource(ResourceID resource_id) = 0;

		virtual bool save_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle resource_handle) = 0;
		virtual ResourceHandle load_resource(Vadon::Utilities::Serializer& serializer) = 0;

		virtual bool serialize_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, ResourceHandle& property_value) = 0;

		virtual void remove_resource(ResourceHandle resource_handle) = 0;

		template<typename T>
		const T* get_resource(TypedResourceHandle<T> resource_handle) const
		{
			static_assert(std::is_base_of_v<Resource, T>);
			return static_cast<const T*>(get_base_resource(resource_handle));
		}

		template<typename T>
		T* get_resource(TypedResourceHandle<T> resource_handle) { return const_cast<T*>(std::as_const(*this).get_resource<T>(resource_handle)); }

		virtual const Resource* get_base_resource(ResourceHandle resource_handle) const = 0;
		Resource* get_base_resource(ResourceHandle resource_handle) { return const_cast<Resource*>(std::as_const(*this).get_base_resource(resource_handle)); }
	protected:
		ResourceSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif