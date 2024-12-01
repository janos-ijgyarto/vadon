#ifndef VADON_SCENE_RESOURCE_RESOURCE_HPP
#define VADON_SCENE_RESOURCE_RESOURCE_HPP
#include <Vadon/Core/File/Path.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace Vadon::Scene
{
	VADON_DECLARE_TYPED_POOL_HANDLE(Resource, ResourceHandle);
	using ResourceID = Vadon::Utilities::UUID;

	using ResourcePath = Vadon::Core::FileSystemPath;

	struct ResourceInfo
	{
		ResourceID id;
		Vadon::Utilities::TypeID type_id;
		ResourcePath path;
	};

	struct ResourceBase
	{
		ResourceID id;
		std::string name;

		virtual ~ResourceBase() {}
	};

	template<typename T>
	struct TypedResourceHandle : public ResourceHandle
	{
		using _ResourceType = T;

		TypedResourceHandle<T>& operator=(const ResourceHandle& h) { handle = h.handle; return *this; }

		ResourceHandle to_resource_handle() const { return ResourceHandle{ .handle = this->handle }; }
		TypedResourceHandle<T>& from_resource_handle(ResourceHandle h) { this->handle = h.handle; return *this; }

		uint64_t to_uint() const { return this->handle.to_uint(); }
	};
}

#define VADON_DECLARE_TYPED_RESOURCE_HANDLE(_resource, _name) using _name = Vadon::Scene::TypedResourceHandle<_resource>
#endif