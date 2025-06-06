#ifndef VADONDEMO_VIEW_RESOURCE_HPP
#define VADONDEMO_VIEW_RESOURCE_HPP
#include <Vadon/Utilities/Enum/EnumClass.hpp>
#include <Vadon/Utilities/Math/Color.hpp>
#include <Vadon/Render/Canvas/Batch.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/SRV.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/Texture.hpp>
#include <Vadon/Scene/Resource/Resource.hpp>
namespace VadonDemo::View
{
	struct ViewResource : public Vadon::Scene::Resource
	{
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
		Vadon::Utilities::ColorRGBA color = Vadon::Utilities::Color_White;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(Shape, ShapeResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(Shape, ShapeResourceHandle);

	struct Sprite : public ViewResource
	{
		std::string texture_path; // FIXME: implement texture resource!
		Vadon::Render::SRVHandle texture_srv; // FIXME: we shouldn't immediately point to an SRV, have client determine how they want to use texture data

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(Sprite, SpriteResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(Sprite, SpriteResourceHandle);

	struct BackgroundSprite : public Sprite
	{
		bool repeat = false;
		bool rotate = false;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(BackgroundSprite, BackgroundSpriteResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(BackgroundSprite, BackgroundSpriteResourceHandle);
}
#endif