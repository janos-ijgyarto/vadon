#ifndef VADON_SCENE_SCENE_HPP
#define VADON_SCENE_SCENE_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
namespace Vadon::Scene
{
	struct SceneInfo
	{
		std::string name;
		// TODO: scene inheritance?
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Scene, SceneHandle);
}
#endif