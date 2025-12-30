#ifndef VADON_FOUNDATION_TYPEINFO_METADATAREGISTRY_HPP
#define VADON_FOUNDATION_TYPEINFO_METADATAREGISTRY_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
namespace Vadon
{
	namespace Foundation
	{
		class TypeMetadataRegistry
		{
		public:
			virtual size_t get_registered_type_count() const = 0;
			virtual UUID get_type_uuid(size_t index) const = 0;

			virtual size_t get_type_property_count(const UUID& type_uuid) const = 0;
			virtual UUID get_type_property_uuid(const UUID& type_uuid, size_t property_index) const = 0;

			virtual void set_type_metadata(const UUID& type_uuid, const char* key, const char* value) = 0;
			virtual const char* get_type_metadata(const UUID& type_uuid, const char* key) const = 0;

			virtual void set_property_metadata(const UUID& type_uuid, const UUID& property_uuid, const char* key, const char* value) = 0;
			virtual const char* get_property_metadata(const UUID& type_uuid, const UUID& property_uuid, const char* key) const = 0;
		};
	}
}
#endif