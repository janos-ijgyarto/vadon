#ifndef VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_MODEL_HPP
#define VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_MODEL_HPP
#include <Vadon/Foundation/Model/Scene/Scene.hpp>
namespace Vadon
{
	namespace Foundation
	{
		enum class EditorModelMessageType : uint32
		{
			RESOURCE_LOADED,
			RESOURCE_PROPERTY_EDITED,
			RESOURCE_REMOVED,
			RESOURCE_ADD_EMBEDDED,
			RESOURCE_REMOVE_EMBEDDED,
			SCENE_OPENED,
			SCENE_SELECTED,
			SCENE_CLOSED,
			ENTITY_ADDED,
			ENTITY_MODIFIED,
			ENTITY_REMOVED,
			COMPONENT_ADDED,
			COMPONENT_PROPERTY_EDITED,
			COMPONENT_REMOVED
		};

		struct EditorModelMessageHeader
		{
			EditorModelMessageType message_type;
		};

		struct EditorModelMessageResourceLoaded : public EditorModelMessageHeader
		{
			UUID resource_id;
			// TODO: anything else?
		};

		struct EditorModelMessageResourcePropertyEdited : public EditorModelMessageHeader
		{
			UUID resource_id;
			UUID property_id;
			uint32 data_size; // NOTE: only used if the data needed to be serialized to JSON
		};

		struct EditorModelMessageResourceRemoved : public EditorModelMessageHeader
		{
			UUID resource_id;
			// TODO: anything else?
		};

		struct EditorModelMessageResourceAddEmbedded : public EditorModelMessageHeader
		{
			UUID resource_id;
			UUID embedded_id;
			UUID embedded_type_id;
			// TODO: anything else?
		};

		struct EditorModelMessageResourceRemoveEmbedded : public EditorModelMessageHeader
		{
			UUID resource_id;
			UUID embedded_id;
			// TODO: anything else?
		};

		struct EditorModelMessageSceneOpened : public EditorModelMessageHeader
		{
			UUID scene_id;
			// TODO: anything else?
		};

		struct EditorModelMessageSceneSelected : public EditorModelMessageHeader
		{
			UUID scene_id;
			// TODO: anything else?
		};

		struct EditorModelMessageSceneClosed : public EditorModelMessageHeader
		{
			UUID scene_id;
			// TODO: anything else?
		};

		struct EditorModelMessageEntityAdded : public EditorModelMessageHeader
		{
			UUID scene_id;
			SceneEntityInfo entity_info;
			// TODO: anything else?
		};

		struct EditorModelMessageEntityModified : public EditorModelMessageHeader
		{
			UUID scene_id;
			UUID entity_id;
			uint32 entity_name_length;
			// TODO: anything else?
		};

		struct EditorModelMessageEntityRemoved : public EditorModelMessageHeader
		{
			UUID scene_id;
			UUID entity_id;
			// TODO: anything else?
		};

		struct EditorModelMessageComponentAdded : public EditorModelMessageHeader
		{
			UUID scene_id;
			UUID entity_id;
			UUID component_id;
			// TODO: anything else?
		};

		struct EditorModelMessageComponentPropertyEdited : public EditorModelMessageHeader
		{
			UUID scene_id;
			UUID entity_id;
			UUID component_id;
			UUID property_id;
			uint32 data_size;
		};

		struct EditorModelMessageComponentRemoved : public EditorModelMessageHeader
		{
			UUID scene_id;
			UUID entity_id;
			UUID component_id;
			// TODO: anything else?
		};
	}
}
#endif