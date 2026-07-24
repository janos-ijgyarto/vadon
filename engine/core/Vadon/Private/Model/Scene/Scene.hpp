#ifndef VADON_PRIVATE_MODEL_SCENE_SCENE_HPP
#define VADON_PRIVATE_MODEL_SCENE_SCENE_HPP
#include <Vadon/Private/Model/Resource/Resource.hpp>
#include <Vadon/Model/Scene/Scene.hpp>
#include <Vadon/Utilities/Data/Object.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>
namespace Vadon::Foundation
{
	class TypeMetadataRegistry;
}
namespace Vadon::Private::Model
{
	using Scene = Vadon::Model::Scene;
	using SceneID = Vadon::Model::SceneID;
	using SceneHandle = Vadon::Model::SceneHandle;

	using EntitySceneInfo = Vadon::Model::EntitySceneInfo;
	using SceneComponent = Vadon::Model::SceneComponent;

	using ComponentData = Vadon::Utilities::ObjectWrapper;

	struct EntityData
	{
		::Vadon::Foundation::UUID id;
		::Vadon::Foundation::UUID parent;
		SceneID scene;
		std::vector<ComponentData> components; // FIXME: use DataObject to store property data instead of allocating Component objects?
		std::string name; // FIXME: this is only needed for the Editor, should be removed in live builds!

		VADON_DECLARE_MEMBER_UUID(id, ::Vadon::Foundation::SceneEntitySchema::c_id_property.id.string);
		VADON_DECLARE_MEMBER_UUID(parent, ::Vadon::Foundation::SceneEntitySchema::c_parent_property.id.string);
		VADON_DECLARE_MEMBER_UUID(scene, ::Vadon::Foundation::SceneEntitySchema::c_scene_property.id.string);
		VADON_DECLARE_MEMBER_UUID(components, ::Vadon::Foundation::SceneEntitySchema::c_components_property.id.string);
		VADON_DECLARE_MEMBER_UUID(name, ::Vadon::Foundation::SceneEntitySchema::c_name_property.id.string);

		bool has_parent() const { return parent.is_valid(); }

		void set_components(const std::vector<ComponentData>& component_vec);
		void clear_component_data();
	};
}

VADON_REGISTER_TYPE_UUID(Vadon::Private::Model::EntityData, ::Vadon::Foundation::SceneEntitySchema::c_type_uuid.string);

VADON_DEFINE_OBJECT_VARIANT_TYPE_TRAIT(Vadon::Private::Model::EntityData);
VADON_DEFINE_OBJECT_TYPE_ERASURE(Vadon::Private::Model::EntityData);

namespace Vadon::Model
{
	struct Scene : public Resource
	{
		~Scene();

		std::vector<Vadon::Private::Model::EntityData> entities;

		VADON_DECLARE_MEMBER_UUID(entities, ::Vadon::Foundation::SceneSchema::c_entities_property.id.string);

		static void register_type_info();
		static void register_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};
}
#endif