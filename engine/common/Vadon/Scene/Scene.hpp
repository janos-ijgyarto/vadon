#ifndef VADON_SCENE_SCENE_HPP
#define VADON_SCENE_SCENE_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
namespace Vadon::Scene
{
	struct SceneInfo
	{
		std::string name;
		// TODO: scene inheritance?

		void swap(SceneInfo& other)
		{
			name.swap(other.name);
		}
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Scene, SceneHandle);
}
#endif