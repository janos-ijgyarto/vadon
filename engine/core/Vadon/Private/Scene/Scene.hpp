#ifndef VADON_PRIVATE_SCENE_SCENE_HPP
#define VADON_PRIVATE_SCENE_SCENE_HPP
#include <Vadon/Private/Scene/Resource/Resource.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Utilities/Data/Variant.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/ErasedDataType.hpp>
namespace Vadon::Private::Scene
{
	using Scene = Vadon::Scene::Scene;

	using SceneHandle = Vadon::Scene::SceneHandle;

	using SceneComponent = Vadon::Scene::SceneComponent;

	struct SceneData
	{
		struct ComponentData
		{
			struct Property
			{
				std::string name;
				Vadon::Utilities::Variant value;
				Vadon::Utilities::ErasedDataTypeID data_type;
			};

			Vadon::Utilities::TypeID type_id = Vadon::Utilities::TypeID::INVALID;
			std::vector<Property> properties;
		};

		struct EntityData
		{
			std::string name;
			int32_t parent = -1;
			SceneHandle scene;
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
#endif