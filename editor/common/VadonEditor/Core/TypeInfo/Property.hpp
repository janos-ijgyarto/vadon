#ifndef VADONEDITOR_CORE_TYPEINFO_PROPERTY_HPP
#define VADONEDITOR_CORE_TYPEINFO_PROPERTY_HPP
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>
namespace VadonEditor::Core
{
	enum class PropertyDataType
	{
		TRIVIAL,
		RESOURCE_ID,
		ARRAY
	};

	struct PropertyTypeID
	{
		PropertyDataType category;
		Vadon::Utilities::TypeID type_id;
	};

	struct Property
	{
		Vadon::Utilities::PropertyUUID id;
		PropertyTypeID type;
		std::string label;
		Vadon::Utilities::Variant value;
	};
}
#endif