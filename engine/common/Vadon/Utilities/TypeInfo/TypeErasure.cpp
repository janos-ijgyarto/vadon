#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/TypeInfo/TypeErasure.hpp>

namespace
{
	Vadon::Utilities::Variant get_trivial_type_default_value(const Vadon::Utilities::ErasedDataTypeID& type_id)
	{
		switch (type_id.id)
		{
		case Vadon::Utilities::type_list_index_v<int, Vadon::Utilities::Variant>:
			return Vadon::Utilities::Variant(0);
		case Vadon::Utilities::type_list_index_v<float, Vadon::Utilities::Variant>:
			return Vadon::Utilities::Variant(0.0f);
		case Vadon::Utilities::type_list_index_v<bool, Vadon::Utilities::Variant>:
			return Vadon::Utilities::Variant(false);
		case Vadon::Utilities::type_list_index_v<std::string, Vadon::Utilities::Variant>:
			return Vadon::Utilities::Variant(std::string());
		case Vadon::Utilities::type_list_index_v<Vadon::Math::Vector2, Vadon::Utilities::Variant>:
			return Vadon::Utilities::Variant(Vadon::Math::Vector2_Zero);
		case Vadon::Utilities::type_list_index_v<Vadon::Math::Vector2i, Vadon::Utilities::Variant>:
			return Vadon::Utilities::Variant(Vadon::Math::Vector2i{0, 0});
		case Vadon::Utilities::type_list_index_v<Vadon::Math::Vector3, Vadon::Utilities::Variant>:
			return Vadon::Utilities::Variant(Vadon::Math::Vector3_Zero);
		case Vadon::Utilities::type_list_index_v<Vadon::Math::Vector3i, Vadon::Utilities::Variant>:
			return Vadon::Utilities::Variant(Vadon::Math::Vector3i{0, 0, 0});
		case Vadon::Utilities::type_list_index_v<Vadon::Math::Vector4, Vadon::Utilities::Variant>:
			return Vadon::Utilities::Variant(Vadon::Math::Vector4_Zero);
		case Vadon::Utilities::type_list_index_v<Vadon::Math::ColorRGBA, Vadon::Utilities::Variant>:
			return Vadon::Utilities::Variant(Vadon::Math::Color_White);
		default:
			break;
		}

		VADON_UNREACHABLE;
	}
}

namespace Vadon::Utilities
{
	Variant get_erased_type_default_value(const ErasedDataTypeID& type_id)
	{
		switch (type_id.type)
		{
		case ErasedDataType::TRIVIAL:
			return get_trivial_type_default_value(type_id);
		case ErasedDataType::RESOURCE_ID:
			return Variant(Vadon::Scene::ResourceID());
		// NOTE: for containers, we expect client code to drill down to value types and request default values
		default:
			break;
		}

		VADON_UNREACHABLE;
	}
}