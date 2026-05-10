#ifndef VADON_PRIVATE_MODEL_SCENE_SCENE_HPP
#define VADON_PRIVATE_MODEL_SCENE_SCENE_HPP
#include <Vadon/Private/Model/Resource/Resource.hpp>
#include <Vadon/Model/Scene/Scene.hpp>
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

	using SceneComponent = Vadon::Model::SceneComponent;

	struct SceneData
	{
		struct ComponentData
		{
			Vadon::Utilities::TypeID type_id = Vadon::Utilities::TypeID::INVALID;
			std::vector<Vadon::Utilities::Property> properties;
		};

		struct EntityData
		{
			int32_t parent = -1;
			SceneID scene;
			std::vector<ComponentData> components;

			bool has_parent() const { return (parent >= 0); }
		};

		// FIXME: optimize by splitting into arrays for data/strings/etc?
		std::vector<EntityData> entities;

		void clear()
		{
			entities.clear();
		}

		void swap(SceneData& other)
		{
			entities.swap(other.entities);
		}
	};
}
namespace Vadon::Model
{
	struct Scene : public Resource
	{
		Vadon::Private::Model::SceneData data;

		static void register_type_info();
		static void register_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};
}
#endif