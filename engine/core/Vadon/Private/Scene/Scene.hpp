#ifndef VADON_PRIVATE_SCENE_SCENE_HPP
#define VADON_PRIVATE_SCENE_SCENE_HPP
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Private/Core/Object/Variant.hpp>
namespace Vadon::Private::Scene
{
	using SceneInfo = Vadon::Scene::SceneInfo;

	using SceneHandle = Vadon::Scene::SceneHandle;

	struct SceneData
	{
		SceneInfo info;

		struct NodeData
		{
			struct Property
			{
				std::string name;
				Core::Variant value;
			};

			std::string name;
			int32_t parent = 0;
			// TODO: child index?
			std::string type;
			std::vector<Property> properties;
		};

		// FIXME: optimize by splitting into arrays for data/strings/etc?
		std::vector<NodeData> nodes;

		void clear()
		{
			nodes.clear();
		}

		void swap(SceneData& other)
		{
			nodes.swap(other.nodes);
		}
	};
}
#endif