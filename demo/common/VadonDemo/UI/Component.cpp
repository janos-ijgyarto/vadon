#include <VadonDemo/UI/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::UI
{
	void Base::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Base>();

		Vadon::Utilities::TypeRegistry::add_property<Base>(VADON_GET_MEMBER_UUID(Base, enabled), Vadon::Utilities::MemberVariableBind<&Base::enabled>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Base>(VADON_GET_MEMBER_UUID(Base, position), Vadon::Utilities::MemberVariableBind<&Base::position>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Base>(VADON_GET_MEMBER_UUID(Base, dimensions), Vadon::Utilities::MemberVariableBind<&Base::dimensions>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(Base))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::UI::Base")
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/UI")
			.add_property(VADON_GET_MEMBER_UUID(Base, enabled))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Enabled")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(Base, position))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Position")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(Base, dimensions))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Dimensions")
				.commit_property();
	}

	void Frame::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Frame>();

		Vadon::Utilities::TypeRegistry::add_property<Frame>(VADON_GET_MEMBER_UUID(Frame, outline_color), Vadon::Utilities::MemberVariableBind<&Frame::outline_color>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Frame>(VADON_GET_MEMBER_UUID(Frame, outline_thickness), Vadon::Utilities::MemberVariableBind<&Frame::outline_thickness>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Frame>(VADON_GET_MEMBER_UUID(Frame, fill_color), Vadon::Utilities::MemberVariableBind<&Frame::fill_color>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Frame>(VADON_GET_MEMBER_UUID(Frame, fill_enable), Vadon::Utilities::MemberVariableBind<&Frame::fill_enable>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(Frame))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::UI::Frame")
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/UI")
			.add_property(VADON_GET_MEMBER_UUID(Frame, outline_color))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Outline Color")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(Frame, outline_thickness))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Outline Thickness")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(Frame, fill_color))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Fill Color")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(Frame, fill_enable))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Fill Enable")
				.commit_property();
	}

	void Text::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Text>();

		Vadon::Utilities::TypeRegistry::add_property<Text>(VADON_GET_MEMBER_UUID(Text, text), Vadon::Utilities::MemberVariableBind<&Text::text>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Text>(VADON_GET_MEMBER_UUID(Text, color), Vadon::Utilities::MemberVariableBind<&Text::color>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Text>(VADON_GET_MEMBER_UUID(Text, offset), Vadon::Utilities::MemberVariableBind<&Text::offset>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(Text))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::UI::Text")
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/UI")
			.add_property(VADON_GET_MEMBER_UUID(Text, text))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Text")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(Text, color))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Color")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(Text, offset))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Offset")
				.commit_property();
	}

	void Selectable::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Selectable>();

		Vadon::Utilities::TypeRegistry::add_property<Selectable>(VADON_GET_MEMBER_UUID(Selectable, clicked_key), Vadon::Utilities::MemberVariableBind<&Selectable::clicked_key>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(Selectable))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::UI::Selectable")
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/UI")
			.add_property(VADON_GET_MEMBER_UUID(Selectable, clicked_key))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Clicked Key")
				.commit_property();
	}
}