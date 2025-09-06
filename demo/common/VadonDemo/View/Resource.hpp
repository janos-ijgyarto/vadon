#ifndef VADONDEMO_VIEW_RESOURCE_HPP
#define VADONDEMO_VIEW_RESOURCE_HPP
#include <VadonDemo/Render/Resource.hpp>
#include <Vadon/Math/Color.hpp>
#include <Vadon/Utilities/Enum/EnumClass.hpp>
#include <Vadon/Render/Canvas/Batch.hpp>
namespace VadonDemo::View
{
	struct RenderResource : public Vadon::Scene::Resource
	{
		Vadon::Render::Canvas::BatchHandle batch;
		Vadon::Utilities::DataRange batch_range;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(RenderResource, RenderResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(RenderResource, RenderResourceHandle);

	enum class ShapeType : int
	{
		TRIANGLE,
		RECTANGLE,
		DIAMOND,
		CIRCLE
	};

	struct Shape : public RenderResource
	{
		int type = Vadon::Utilities::to_integral(ShapeType::TRIANGLE); // FIXME: placeholder solution, need better way to data-drive drawable objects!
		float radius = 1.0f;

		// TODO: create material resource to unify this (color + texture + anything else)?
		Vadon::Math::ColorRGBA color = Vadon::Math::Color_White;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(Shape, ShapeResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(Shape, ShapeResourceHandle);

	struct Sprite : public RenderResource
	{
		// TODO: create material resource to unify this (color + texture + anything else)?
		// TODO2: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		VadonDemo::Render::TextureResourceID texture;
		// TODO: additional properties?

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(Sprite, SpriteResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(Sprite, SpriteResourceHandle);
}
#endif