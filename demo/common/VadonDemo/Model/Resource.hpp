#ifndef VADONDEMO_MODEL_RESOURCE_HPP
#define VADONDEMO_MODEL_RESOURCE_HPP
#include <Vadon/Utilities/Enum/EnumClass.hpp>
#include <Vadon/Render/Canvas/Item.hpp>
#include <Vadon/Scene/Resource/Resource.hpp>
namespace VadonDemo::Model
{
	// TODO: implement system that uses resources to allow basic shapes
	// More complex shapes will use an advanced system that generates visuals based on game state
	enum class CanvasItemType
	{
		TRIANGLE,
		BOX,
		DIAMOND
		// TODO: other shapes!
	};

	struct CanvasItemDefinition : public Vadon::Scene::Resource
	{
		int type = Vadon::Utilities::to_integral(CanvasItemType::TRIANGLE); // FIXME: implement a way to do enums, list of selectable options, etc.
		Vadon::Utilities::Vector3 color = Vadon::Utilities::Vector3_One;
		float z_order = 0.0f;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(CanvasItemDefinition, CanvasItemDefID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(CanvasItemDefinition, CanvasItemDefHandle);
}
#endif