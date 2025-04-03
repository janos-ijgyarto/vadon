#ifndef VADONDEMO_CORE_COMPONENT_HPP
#define VADONDEMO_CORE_COMPONENT_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>
namespace VadonDemo::Core
{
	// FIXME: could backport this to the Project API and store a LUT of resources
	// Maybe copy Godot's Autoload system
	struct CoreComponent
	{
		Vadon::Scene::SceneHandle main_menu;
		Vadon::Scene::SceneID default_start_level;
		Vadon::Utilities::Vector2 viewport_size = Vadon::Utilities::Vector2_One;

		static void register_component();
	};
}
#endif