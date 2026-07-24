#ifndef VADON_FOUNDATION_TYPEINFO_METADATA_HPP
#define VADON_FOUNDATION_TYPEINFO_METADATA_HPP
#include <Vadon/Foundation/Utilities/Numeric.hpp>
#include <cstring>
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
				FLAGS,		
				// TODO: other metadata?
			};

			static constexpr const char* key_string(Key key)
			{
				switch (key)
				{
				case NAME:
					return "name";
				case FLAGS:
					return "flags";
				}

				return nullptr;
			}

			enum Flags
			{
				NONE = 0,
				EDITOR_HIDDEN = 1 << 0 // Property not displayed in Editor UI
			};

			static constexpr const char* flag_string(Flags flags)
			{
				for (uint32 current_offset = 0u; (1 << current_offset) <= flags; ++current_offset)
				{
					const uint32 current_flag_value = 1 << current_offset;
					if ((flags & current_flag_value) != 0)
					{
						switch (current_flag_value)
						{
						case Flags::EDITOR_HIDDEN:
							return "editor_hidden";
						}
					}
				}

				return nullptr;
			}

			static Flags parse_flag_string(const char* flag_string)
			{
				// FIXME: make this cleaner!
				if (strcmp(flag_string, "editor_hidden") == 0)
				{
					return Flags::EDITOR_HIDDEN;
				}

				return Flags::NONE;
			}
		};
	}
}
#endif