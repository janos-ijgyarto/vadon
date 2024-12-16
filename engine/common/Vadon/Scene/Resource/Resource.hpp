#ifndef VADON_SCENE_RESOURCE_RESOURCE_HPP
#define VADON_SCENE_RESOURCE_RESOURCE_HPP
#include <Vadon/Core/File/Path.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace Vadon::Scene
{
	VADON_DECLARE_TYPED_POOL_HANDLE(ResourceBase, ResourceHandle);
	using ResourceID = Vadon::Utilities::UUID;

	using ResourcePath = Vadon::Core::FileSystemPath;

	struct ResourceInfo
	{
		ResourceID id;
		Vadon::Utilities::TypeID type_id;
		ResourcePath path;

		bool is_valid() const { return id.is_valid(); }
	};

	struct Resource
	{
		ResourceID id;
		std::string name;

		virtual ~Resource() {}
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
}

#define VADON_DECLARE_TYPED_RESOURCE_HANDLE(_resource, _name) using _name = Vadon::Scene::TypedResourceHandle<_resource>
#endif