#ifndef VADONDEMO_UI_COMPONENT_HPP
#define VADONDEMO_UI_COMPONENT_HPP
#include <Vadon/Render/Canvas/Item.hpp>
#include <Vadon/Render/Utilities/Color.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace Vadon::Foundation
{
	class TypeMetadataRegistry;
}
namespace VadonDemo::UI
{
	// TODO: implement some kind of logic which allows controlling "groups" of UI entities
	// e.g to show/hide all the elements of an entire widget
	struct Base
	{
		VADON_DECLARE_MEMBER_UUID(enabled, "4311fdd6-22fa-4d93-b166-2e077bef092f");
		VADON_DECLARE_MEMBER_UUID(position, "c9ab264e-9cdc-4376-acb7-8b6c9acb54e9");
		VADON_DECLARE_MEMBER_UUID(dimensions, "8b5a8a13-7ea4-435c-ba1e-5a571a2b740a");

		bool enabled = true;

		Vadon::Math::Vector2 position = Vadon::Math::Vector2_Zero;
		Vadon::Math::Vector2 dimensions = Vadon::Math::Vector2_Zero;

		// TODO: position and sizing style, whether to use absolute coords or relative to screen size

		bool dirty = false; // FIXME: replace with editor-specific "tag component" implementation!

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	struct Frame
	{
		VADON_DECLARE_MEMBER_UUID(outline_color, "55cac320-4777-415d-ad56-0eb71fac3c05");
		VADON_DECLARE_MEMBER_UUID(outline_thickness, "30e46815-047e-4f03-b95a-e65bef04bb0d");
		VADON_DECLARE_MEMBER_UUID(fill_color, "8f661f11-b45d-4d79-b04c-db4aea11cc35");
		VADON_DECLARE_MEMBER_UUID(fill_enable, "f09ba9f8-e602-4011-bd8b-556b031d8ff7");

		Vadon::Render::ColorRGBA outline_color = Vadon::Math::Color_White;
		float outline_thickness = 1.0f;

		Vadon::Render::ColorRGBA fill_color = Vadon::Math::Color_White;
		bool fill_enable = false;

		// TODO: draw anchor, to decide whether to draw relative to center or top left corner

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	struct Text
	{
		VADON_DECLARE_MEMBER_UUID(text, "a67b0ede-d770-4026-bdd8-082a374ab0ce");
		VADON_DECLARE_MEMBER_UUID(color, "68c2368d-0eca-436c-a68f-e0a2a8237ba2");
		VADON_DECLARE_MEMBER_UUID(offset, "eda450ae-3a7f-403e-968f-cfd4ad216897");

		std::string text;
		Vadon::Render::ColorRGBA color = Vadon::Math::Color_White;
		Vadon::Math::Vector2 offset = Vadon::Math::Vector2_Zero;
		// TODO: font, text size, etc.
		// TODO: alignment (left vs center)

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	struct Selectable
	{
		VADON_DECLARE_MEMBER_UUID(clicked_key, "d9670d33-d1e4-49c4-b1d0-a2fd10fbbbe6");

		// TODO: more advanced signal/callback system?
		std::string clicked_key;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};
}

VADON_REGISTER_TYPE_UUID(VadonDemo::UI::Base, "e9179c40-3ba4-40bd-afdd-6099b334da20");
VADON_REGISTER_TYPE_UUID(VadonDemo::UI::Frame, "72b1de45-0325-42e1-89b8-0bc8eb2ffdec");
VADON_REGISTER_TYPE_UUID(VadonDemo::UI::Text, "1a49fe09-29a7-4c27-8dbf-4a50b17d8324");
VADON_REGISTER_TYPE_UUID(VadonDemo::UI::Selectable, "1151b7d4-69fe-4db0-878c-6b85d873bed7");
#endif