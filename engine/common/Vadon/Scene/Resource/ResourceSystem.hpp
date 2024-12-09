#ifndef VADON_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#define VADON_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#include <Vadon/Scene/Module.hpp>
#include <Vadon/Scene/Resource/Resource.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>
namespace Vadon::Utilities
{
	class Serializer;
}
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

		virtual ResourceHandle create_resource(Vadon::Utilities::TypeID type_id) = 0;

		virtual ResourceInfo get_resource_info(ResourceHandle resource_handle) const = 0;
		virtual void set_resource_path(ResourceHandle resource_handle, const ResourcePath& path) = 0;

		virtual ResourceHandle find_resource(ResourceID resource_id) const = 0;

		template<typename T>
		std::vector<ResourceHandle> find_resources_of_type() const
		{
			static_assert(std::is_base_of_v<ResourceBase, T>);
			return find_resources_of_type(Vadon::Utilities::TypeRegistry::get_type_id<T>());
		}

		virtual std::vector<ResourceHandle> find_resources_of_type(Vadon::Utilities::TypeID type_id) const = 0;

		virtual ResourceHandle import_resource(const ResourcePath& path) = 0;
		virtual ResourceHandle load_resource(const ResourcePath& path) = 0;

		virtual bool save_resource(ResourceHandle resource_handle) = 0;
		virtual bool load_resource(ResourceHandle resource_handle) = 0;

		virtual bool serialize_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle& resource_handle) = 0;
		virtual bool serialize_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, ResourceHandle& property_value) = 0;

		virtual void unload_resource(ResourceHandle resource_handle) = 0;
		virtual void remove_resource(ResourceHandle resource_handle) = 0;

		virtual bool is_resource_loaded(ResourceHandle resource_handle) const = 0;

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
	protected:
		ResourceSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif