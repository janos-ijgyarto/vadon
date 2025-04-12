#ifndef VADON_PRIVATE_SCENE_SCENE_HPP
#define VADON_PRIVATE_SCENE_SCENE_HPP
#include <Vadon/Private/Scene/Resource/Resource.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>
namespace Vadon::Private::Scene
{
	using Scene = Vadon::Scene::Scene;
	using SceneID = Vadon::Scene::SceneID;
	using SceneHandle = Vadon::Scene::SceneHandle;

	using SceneComponent = Vadon::Scene::SceneComponent;

	struct SceneData
	{
		struct ComponentData
		{
			Vadon::Utilities::TypeID type_id = Vadon::Utilities::TypeID::INVALID;
			std::vector<Vadon::Utilities::Property> properties;
		};

		struct EntityData
		{
			std::string name;
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

		static void register_scene_type_info();
	};
}
namespace Vadon::Scene
{
	struct Scene : public Resource
	{
		Vadon::Private::Scene::SceneData data;
	};
}
#endif