#ifndef VADON_UTILITIES_TYPEINFO_METADATA_HPP
#define VADON_UTILITIES_TYPEINFO_METADATA_HPP
#include <Vadon/Foundation/TypeInfo/Metadata.hpp>
#include <Vadon/Foundation/TypeInfo/MetadataRegistry.hpp>
#include <Vadon/Utilities/TypeInfo/Registry.hpp>
namespace Vadon::Utilities
{
	// TODO: this could be moved to the MetadataRegistry
	// - Set functor in registry for getting type/property info
	// - When registering type, functor is called to get info

	// NOTE: helper classes to make it less cumbersome to register metadata for types and their properties
	struct TypePropertyMetadata;
	struct TypeMetadataBase
	{
		::Vadon::Foundation::TypeMetadataRegistry& registry;
		const Vadon::Utilities::TypeID id;
		const ::Vadon::Foundation::TypeInfo type_info;

		// NOTE: using TypeID to ensure it's a registered type
		TypeMetadataBase(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry, const Vadon::Utilities::TypeID type_id)
			: registry(metadata_registry)
			, id(type_id)
			, type_info(Vadon::Utilities::TypeRegistry::get_type_info(type_id))
		{
			registry.register_type(type_info);
		}

		void set_metadata(const char* key, const char* value)
		{
			registry.set_type_metadata(type_info.id, key, value);
		}

		TypeMetadataBase& add_metadata(const char* key, const char* value)
		{
			set_metadata(key, value);
			return *this;
		}

		void set_metadata(::Vadon::Foundation::CommonTypeMetadata::Key key, const char* value)
		{
			set_metadata(::Vadon::Foundation::CommonTypeMetadata::key_string(key), value);
		}

		TypeMetadataBase& add_metadata(::Vadon::Foundation::CommonTypeMetadata::Key key, const char* value)
		{
			set_metadata(key, value);
			return *this;
		}

		TypePropertyMetadata add_property(const ::Vadon::Foundation::UUID& property_uuid);
	};

	template<typename T>
	class TypeMetadata : public TypeMetadataBase
	{
	public:
		TypeMetadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
			: TypeMetadataBase(metadata_registry, Vadon::Utilities::TypeRegistry::get_type_id<T>())
		{

		}
	};

	// TODO: some template trickery to get a TypeMetadata object via the type itself

	struct TypePropertyMetadata
	{
		TypeMetadataBase& type_metadata;
		const ::Vadon::Utilities::TypeUUID uuid;

		TypePropertyMetadata(TypeMetadataBase& type_metadata_obj, const ::Vadon::Foundation::UUID& property_uuid)
			: type_metadata(type_metadata_obj)
			, uuid(property_uuid)
		{
			const Vadon::Utilities::PropertyInfo property_info = Vadon::Utilities::TypeRegistry::get_property_info(type_metadata.id, property_uuid);
			type_metadata.registry.register_property(type_metadata.type_info.id, property_info.base_info, property_info.type_list.data());
		}

		void set_metadata(const char* key, const char* value)
		{
			type_metadata.registry.set_property_metadata(type_metadata.type_info.id, uuid, key, value);
		}

		TypePropertyMetadata& add_metadata(const char* key, const char* value) { set_metadata(key, value); return *this; }

		void set_metadata(::Vadon::Foundation::CommonPropertyMetadata::Key key, const char* value)
		{
			set_metadata(::Vadon::Foundation::CommonPropertyMetadata::key_string(key), value);
		}

		TypePropertyMetadata& add_metadata(::Vadon::Foundation::CommonPropertyMetadata::Key key, const char* value) { set_metadata(key, value); return *this; }

		// NOTE: this is intended as the "end" for the initialization sequence, returning to the TypeMetadata to add other metadata/properties
		TypeMetadataBase& commit_property() { return type_metadata; }
	};

	inline TypePropertyMetadata TypeMetadataBase::add_property(const ::Vadon::Foundation::UUID& property_uuid)
	{
		return TypePropertyMetadata(*this, property_uuid);
	}
}
#endif