#ifndef VADON_SCENE_RESOURCE_DATABASE_HPP
#define VADON_SCENE_RESOURCE_DATABASE_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
namespace Vadon::Scene
{
	class ResourceSystem;

	// Used by ResourceSystem to save/load resources
	// Expected to search for a file or some other source,
	// and then call the serialization functions of ResourceSystem
	class ResourceDatabase
	{
	public:
		// TODO: function to find resources of a certain type ID?
		// TODO2: function to check a path and see what resource type is there (if any)?
		// TODO3: function to get resource path (if available, could be scene local)
		virtual bool save_resource(ResourceSystem& resource_system, ResourceHandle resource_handle) = 0;
		virtual ResourceHandle load_resource(ResourceSystem& resource_system, ResourceID resource_id) = 0;
	};
}
#endif