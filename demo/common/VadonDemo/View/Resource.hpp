#ifndef VADONDEMO_VIEW_RESOURCE_HPP
#define VADONDEMO_VIEW_RESOURCE_HPP
#include <Vadon/Utilities/Enum/EnumClass.hpp>
#include <Vadon/Render/Canvas/Batch.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/SRV.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/Texture.hpp>
#include <Vadon/Scene/Resource/Resource.hpp>
namespace VadonDemo::View
{
	struct ViewResource : public Vadon::Scene::Resource
	{
		float z_order = 0.0f; // FIXME: revise this, we need to be able to set Z order per-object!

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(ViewResource, ViewResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(ViewResource, ViewResourceHandle);

	enum class ShapeType : int
	{
		TRIANGLE,
		RECTANGLE,
		DIAMOND
	};

	struct Shape : public ViewResource
	{
		int type = Vadon::Utilities::to_integral(ShapeType::TRIANGLE); // FIXME: placeholder solution, need better way to data-drive drawable objects!
		Vadon::Utilities::Vector3 color = Vadon::Utilities::Vector3_One;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(Shape, ShapeResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(Shape, ShapeResourceHandle);

	struct Sprite : public ViewResource
	{
		std::string texture_path; // FIXME: implement texture resource!
		// TODO: other params (offset, etc.)

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(Sprite, SpriteResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(Sprite, SpriteResourceHandle);
}
#endif