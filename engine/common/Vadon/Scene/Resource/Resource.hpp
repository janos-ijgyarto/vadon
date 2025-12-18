#ifndef VADON_SCENE_RESOURCE_RESOURCE_HPP
#define VADON_SCENE_RESOURCE_RESOURCE_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace Vadon::Scene
{
	VADON_DECLARE_TYPED_POOL_HANDLE(ResourceBase, ResourceHandle);
	using ResourceID = Vadon::Utilities::UUID;
		
	struct ResourceInfo
	{
		ResourceID id;
		Vadon::Utilities::TypeID type_id = Vadon::Utilities::TypeID::INVALID;

		bool is_valid() const { return id.is_valid(); }
	};

	// TODO: revise as class to restrict access to base members?
	struct Resource
	{
		ResourceID id;
		std::string name;

		virtual ~Resource() {}
	};

	template<typename T>
	struct TypedResourceID : public ResourceID
	{
		using _ResourceType = T;
		using _TypedID = TypedResourceID<T>;

		TypedResourceID<T>& operator=(const ResourceID& id) { data = id.data; return *this; }

		ResourceID to_resource_id() const { return ResourceID{ .data = this->data }; }
		static TypedResourceID<T> from_resource_id(ResourceID id) { _TypedID typed_id; typed_id.data = id.data; return typed_id; }

		ResourceID& as_resource_id() { return *this; }
	};

	template<typename T>
	struct TypedResourceHandle : public ResourceHandle
	{
		using _ResourceType = T;
		using _TypedHandle = TypedResourceHandle<T>;

		TypedResourceHandle<T>& operator=(const ResourceHandle& h) { handle = h.handle; return *this; }

		ResourceHandle to_resource_handle() const { return ResourceHandle{ .handle = this->handle }; }
		static TypedResourceHandle<T> from_resource_handle(ResourceHandle h) { _TypedHandle typed_handle; typed_handle.handle = h.handle; return typed_handle; }

		uint64_t to_uint() const { return this->handle.to_uint(); }
	};

	template<typename T>
	concept is_resource_id = std::is_base_of_v<Vadon::Scene::ResourceID, T> && (std::is_same_v<Vadon::Scene::ResourceID, T> == false);

	template<typename T>
	concept is_resource_handle = std::is_base_of_v<Vadon::Scene::ResourceHandle, T> && (std::is_same_v<Vadon::Scene::ResourceHandle, T> == false);
}

#define VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(_resource, _name) using _name = Vadon::Scene::TypedResourceID<_resource>
#define VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(_resource, _name) using _name = Vadon::Scene::TypedResourceHandle<_resource>
#define VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(_resource, _id_name, _handle_name) VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(_resource, _id_name); \
																						VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(_resource, _handle_name)
			
#endif