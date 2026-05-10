#ifndef VADON_MODEL_RESOURCE_FILE_HPP
#define VADON_MODEL_RESOURCE_FILE_HPP
#include <Vadon/Foundation/Model/Resource/File.hpp>
#include <Vadon/Model/Resource/Resource.hpp>
namespace Vadon::Model
{
	// NOTE: resources never interact with files directly
	// This resource type is the only exception, it allows other resources to reference
	// files as resources. Other systems are responsible for loading the file data itself.
	struct FileResource : public Resource
	{
		// TODO: metadata for importing?
		// Will most likely do in derived classes
	};

	VADON_MODEL_DECLARE_TYPED_RESOURCE_ID(FileResource, FileResourceID);
	VADON_MODEL_DECLARE_TYPED_RESOURCE_HANDLE(FileResource, FileResourceHandle);
}

VADON_REGISTER_TYPE_UUID(Vadon::Model::FileResource, ::Vadon::Foundation::FileResourceSchema::c_type_uuid.string);
#endif