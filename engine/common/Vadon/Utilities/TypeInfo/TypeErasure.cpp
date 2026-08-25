#include <Vadon/Utilities/TypeInfo/TypeErasure.hpp>

#include <Vadon/Utilities/Debugging/Assert.hpp>

#include <Vadon/Utilities/TypeInfo/Registry.hpp>

namespace Vadon::Utilities
{
	TypeID get_erased_data_type_id(TypeID type_id)
	{
		// FIXME: find a way to delegate this logic to places that need it!
		const TypeID uuid_type_id = Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Foundation::UUID>();
		if (Vadon::Utilities::TypeRegistry::is_base_of(uuid_type_id, type_id) == true)
		{
			return uuid_type_id;
		}

		return type_id;
	}

	Variant get_erased_type_default_value(TypeID type_id)
	{
		// NOTE: this assumes we register the trivial types first!
		const Vadon::Foundation::BaseType base_type = static_cast<Vadon::Foundation::BaseType>(type_id);
		switch (base_type)
		{
		case Vadon::Foundation::BaseType::INT32:
			return Vadon::Utilities::Variant(0);
		case Vadon::Foundation::BaseType::UINT32:
			return Vadon::Utilities::Variant(0u);
		case Vadon::Foundation::BaseType::FLOAT:
			return Vadon::Utilities::Variant(0.0f);
		case Vadon::Foundation::BaseType::BOOL:
			return Vadon::Utilities::Variant(false);
		case Vadon::Foundation::BaseType::STRING:
			return Vadon::Utilities::Variant(std::string());
		case Vadon::Foundation::BaseType::VECTOR2:
			return Vadon::Utilities::Variant(Vadon::Math::Vector2_Zero);
		case Vadon::Foundation::BaseType::VECTOR2I:
			return Vadon::Utilities::Variant(Vadon::Math::Vector2i{ 0, 0 });
		case Vadon::Foundation::BaseType::VECTOR3:
			return Vadon::Utilities::Variant(Vadon::Math::Vector3_Zero);
		case Vadon::Foundation::BaseType::VECTOR3I:
			return Vadon::Utilities::Variant(Vadon::Math::Vector3i{ 0, 0, 0 });
		case Vadon::Foundation::BaseType::VECTOR4:
			return Vadon::Utilities::Variant(Vadon::Math::Vector4_Zero);
		case Vadon::Foundation::BaseType::COLORRGBA:
			return Vadon::Utilities::Variant(Vadon::Math::Color_White);
		default:
			break;
		}

		VADON_UNREACHABLE;
	}
}