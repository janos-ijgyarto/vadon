#ifndef VADON_UTILITIES_TYPEINFO_METADATA_HPP
#define VADON_UTILITIES_TYPEINFO_METADATA_HPP
#include <Vadon/Foundation/TypeInfo/MetadataRegistry.hpp>
#include <Vadon/Utilities/TypeInfo/Registry.hpp>
namespace Vadon::Utilities
{
	// NOTE: helper classes to make it less cumbersome to register metadata for types and their properties
	struct TypeMetadata
	{
		::Vadon::Foundation::TypeMetadataRegistry& registry;
		const Vadon::Utilities::TypeUUID uuid;
		const Vadon::Utilities::TypeID id;

		TypeMetadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry, const ::Vadon::Foundation::UUID& type_uuid)
			: registry(metadata_registry)
			, uuid(type_uuid)
			, id(Vadon::Utilities::TypeRegistry::get_type_id(uuid))
		{
			registry.register_type(Vadon::Utilities::TypeRegistry::get_type_info(id));
		}

		void set_metadata(const char* key, const char* value)
		{
			registry.set_type_metadata(uuid, key, value);
		}
	};

	// TODO: some template trickery to get a TypeMetadata object via the type itself

	struct TypePropertyMetadata
	{
		TypeMetadata& type_metadata;
		const ::Vadon::Utilities::TypeUUID uuid;

		TypePropertyMetadata(TypeMetadata& type_metadata_obj, const ::Vadon::Foundation::UUID& property_uuid)
			: type_metadata(type_metadata_obj)
			, uuid(property_uuid)
		{
			type_metadata.registry.register_property(type_metadata.uuid, Vadon::Utilities::TypeRegistry::get_property_info(type_metadata.id, property_uuid).base_info);
		}

		void set_metadata(const char* key, const char* value)
		{
			type_metadata.registry.set_property_metadata(type_metadata.uuid, uuid, key, value);
		}
	};
}
#endif