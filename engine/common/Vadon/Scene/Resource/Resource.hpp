#ifndef VADON_SCENE_RESOURCE_RESOURCE_HPP
#define VADON_SCENE_RESOURCE_RESOURCE_HPP
#include <Vadon/Core/File/RootDirectory.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace Vadon::Scene
{
	VADON_DECLARE_TYPED_POOL_HANDLE(Resource, ResourceHandle);
	using ResourceID = Vadon::Utilities::UUID;

	struct ResourceBase
	{
		ResourceID id;
		std::string name;

		virtual ~ResourceBase() {}
	};

	struct ResourcePath
	{
		std::string path;
		Vadon::Core::RootDirectoryHandle root_directory;

		bool is_valid() const { return path.empty() == false; }

		bool operator==(const ResourcePath& other) const { return (path == other.path) && (root_directory == other.root_directory); }
	};

	template<typename T>
	struct TypedResourceHandle : public ResourceHandle
	{
		using _ResourceType = T;
	};
}

#define VADON_DECLARE_TYPED_RESOURCE_HANDLE(_resource, _name) using _name = Vadon::Scene::TypedResourceHandle<_resource>
#endif