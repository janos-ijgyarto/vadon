#ifndef VADON_FOUNDATION_TYPEINFO_PROPERTY_HPP
#define VADON_FOUNDATION_TYPEINFO_PROPERTY_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
namespace Vadon
{
	namespace Foundation
	{
		struct Property
		{
			enum Category
			{
				TRIVIAL, // Maps directly onto base type
				ARRAY, // Array of underlying type (generic if not specified)
				DICTIONARY, // Dictionary of underlying type (generic if not specified)
				OBJECT, // Object of underlying type (generic if not specified)
				RESOURCE, // UUID of Resource
				GENERIC, // Data type is determined at runtime
				CATEGORY_COUNT,
				INVALID = CATEGORY_COUNT
			};

			UUID id;
			UUID root_type;
			size_t type_list_length = 0;
			// TODO: any other metadata?

			bool is_valid() const { return (id.is_valid() == true) && (root_type.is_valid() == true) && (type_list_length != 0); }
		};

		struct PropertySchema
		{
			UUIDString id;
			UUIDString type;
		};
	}
}
#endif