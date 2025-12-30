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
		VADON_DECLARE_MEMBER_UUID(type, "1d2e10fd-4025-4644-913d-55e1cbd13256");
		VADON_DECLARE_MEMBER_UUID(radius, "5c68620a-6bf0-49ec-aa91-2d59c68b5f3b");
		VADON_DECLARE_MEMBER_UUID(color, "371cdb5b-6795-46cc-8aae-0cb5e6140f5c");

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
		VADON_DECLARE_MEMBER_UUID(texture, "522cb89e-efdf-4581-af9d-1696ecf33280");

		// TODO: create material resource to unify this (color + texture + anything else)?
		// TODO2: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		VadonDemo::Render::TextureResourceID texture;
		// TODO: additional properties?

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(Sprite, SpriteResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(Sprite, SpriteResourceHandle);
}

VADON_REGISTER_TYPE_UUID(VadonDemo::View::RenderResource, "8582958a-2b83-4d47-9e39-be1dda427e8c");
VADON_REGISTER_TYPE_UUID(VadonDemo::View::Shape, "d7807fed-a0fb-4acd-b032-c421c59b875a");
VADON_REGISTER_TYPE_UUID(VadonDemo::View::Sprite, "0c33eaba-0d02-4db9-a837-4442a43eba1a");
#endif