#ifndef VADON_FOUNDATION_MODEL_RESOURCE_FILE_HPP
#define VADON_FOUNDATION_MODEL_RESOURCE_FILE_HPP
#include <Vadon/Foundation/TypeInfo/Property.hpp>
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
namespace Vadon
{
	namespace Foundation
	{
		struct FileResourceSchema
		{
			static constexpr UUIDString c_type_uuid{ "af09a2a9-b4eb-4a64-b654-0f7b8a1c0ffb" };
		};

		struct ResourceFileInfo
		{
			enum Type
			{
				RESOURCE,
				SCENE,
				IMPORTED_FILE,
				NONE
			};

			static const char* get_file_extension(Type type)
			{
				switch (type)
				{
				case Type::RESOURCE:
					return "vdrc";
				case Type::SCENE:
					return "vdsc";
				case Type::IMPORTED_FILE:
					return "vdimport";
				}

				return "";
			}
		};
	}
}
#endif