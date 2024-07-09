#ifndef VADON_PRIVATE_SCENE_SCENE_HPP
#define VADON_PRIVATE_SCENE_SCENE_HPP
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Utilities/Data/Variant.hpp>
namespace Vadon::Private::Scene
{
	using SceneInfo = Vadon::Scene::SceneInfo;

	using SceneHandle = Vadon::Scene::SceneHandle;

	struct SceneData
	{
		SceneInfo info;

		struct ComponentData
		{
			struct Property
			{
				std::string name;
				Vadon::Utilities::Variant value;
			};

			std::string type;
			std::vector<Property> properties;
		};

		struct EntityData
		{
			std::string name;
			int32_t parent = -1;
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
			info.swap(other.info);
			entities.swap(other.entities);
		}
	};
}
#endif