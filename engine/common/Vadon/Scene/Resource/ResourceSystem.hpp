#ifndef VADON_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#define VADON_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#include <Vadon/Core/File/File.hpp>
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
		virtual ResourceHandle load_resource_base(ResourceID resource_id) = 0;

		template<typename T>
		TypedResourceHandle<T> load_resource(TypedResourceID<T> resource_id)
		{
			static_assert(std::is_base_of_v<Resource, T>);
			return TypedResourceHandle<T>::from_resource_handle(load_resource_base(resource_id));
		}

		virtual bool save_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle resource_handle) = 0;
		virtual ResourceHandle load_resource(Vadon::Utilities::Serializer& serializer) = 0;

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

		// NOTE: this expects a separate file that is referenced by a FileResource
		virtual Vadon::Core::FileInfo get_file_resource_info(ResourceID resource_id) const = 0;
		virtual bool load_file_resource_data(ResourceID resource_id, Vadon::Core::RawFileDataBuffer& file_data) = 0;
	protected:
		ResourceSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif