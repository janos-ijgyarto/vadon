#ifndef VADON_SCENE_RESOURCE_FILE_HPP
#define VADON_SCENE_RESOURCE_FILE_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
namespace Vadon::Scene
{
	// NOTE: resources never interact with files directly
	// This resource type is the only exception, it allows other resources to reference
	// files as resources. Other systems are responsible for loading the file data itself.
	struct FileResource : public Resource
	{
		// TODO: metadata for importing?
		// Will most likely do in derived classes
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(FileResource, FileResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(FileResource, FileResourceHandle);
}

VADON_REGISTER_TYPE_UUID(Vadon::Scene::FileResource, "af09a2a9-b4eb-4a64-b654-0f7b8a1c0ffb");
#endif