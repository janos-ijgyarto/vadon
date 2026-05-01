#ifndef VADON_FOUNDATION_TYPEINFO_METADATA_HPP
#define VADON_FOUNDATION_TYPEINFO_METADATA_HPP
namespace Vadon
{
	namespace Foundation
	{
		struct CommonTypeMetadata
		{
			enum Key
			{
				NAME,
				COMPONENT
				// TODO: other metadata?
			};

			static constexpr const char* key_string(Key key)
			{
				switch (key)
				{
				case NAME:
					return "name";
				case COMPONENT:
					return "component";
				}

				return nullptr;
			}
		};

		struct CommonPropertyMetadata
		{
			enum Key
			{
				NAME,
				RESOURCE_TYPE, // If property is UUID, this indicates that it should be interpreted as a resource UUID and provides the resource type
				ARRAY_TYPE // If property is array, this contains the type of the array
				// TODO: other metadata?
			};

			static constexpr const char* key_string(Key key)
			{
				switch (key)
				{
				case NAME:
					return "name";
				case RESOURCE_TYPE:
					return "resource_type";
				case ARRAY_TYPE:
					return "array_type";
				}

				return nullptr;
			}
		};
	}
}
#endif