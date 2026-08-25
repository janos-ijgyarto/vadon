#include <VadonEditor/Core/TypeInfo/MetadataRegistry.hpp>

namespace
{
	struct PropertyMetadata
	{
		std::unordered_map<std::string, std::string> metadata_lookup;
	};

	struct TypeMetadata
	{
		std::unordered_map<std::string, std::string> metadata_lookup;
		std::unordered_map<Vadon::Utilities::PropertyUUID, PropertyMetadata> property_lookup;
	};
}

namespace VadonEditor::Core
{
	struct MetadataRegistry::Internal
	{
		std::unordered_map<Vadon::Utilities::TypeUUID, TypeMetadata> m_type_metadata_lookup;
	};

	MetadataRegistry::~MetadataRegistry() = default;

	size_t MetadataRegistry::get_registered_type_count() const
	{
		const std::vector<Vadon::Utilities::TypeUUID> type_uuids = Vadon::Utilities::TypeRegistry::get_all_registered_types();
		return type_uuids.size();
	}

	::Vadon::Foundation::UUID MetadataRegistry::get_type_uuid(size_t index) const
	{
		// FIXME: this is not very efficient, maybe pre-cache the list?
		const std::vector<Vadon::Utilities::TypeUUID> type_uuids = Vadon::Utilities::TypeRegistry::get_all_registered_types();
		return type_uuids[index];
	}

	size_t MetadataRegistry::get_type_property_count(const ::Vadon::Foundation::UUID& type_uuid) const
	{
		const Vadon::Utilities::PropertyInfoList type_properties = Vadon::Utilities::TypeRegistry::get_type_properties(Vadon::Utilities::TypeRegistry::get_type_id(type_uuid));
		return type_properties.size();
	}

	::Vadon::Foundation::UUID MetadataRegistry::get_type_property_uuid(const ::Vadon::Foundation::UUID& type_uuid, size_t property_index) const
	{
		const Vadon::Utilities::PropertyInfoList type_properties = Vadon::Utilities::TypeRegistry::get_type_properties(Vadon::Utilities::TypeRegistry::get_type_id(type_uuid));
		return type_properties[property_index].base_info.id;
	}

	void MetadataRegistry::set_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key, const char* value)
	{
		VADON_ASSERT(type_uuid.is_valid(), "Invalid type UUID!");
		VADON_ASSERT(key != nullptr, "Invalid key!");

		auto type_metadata_it = m_internal->m_type_metadata_lookup.find(type_uuid);
		if (type_metadata_it == m_internal->m_type_metadata_lookup.end())
		{
			type_metadata_it = m_internal->m_type_metadata_lookup.insert(std::make_pair(type_uuid, TypeMetadata())).first;
		}

		TypeMetadata& type_metadata = type_metadata_it->second;
		type_metadata.metadata_lookup[key] = value;
	}

	const char* MetadataRegistry::get_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key) const
	{
		VADON_ASSERT(type_uuid.is_valid(), "Invalid type UUID!");
		auto type_metadata_it = m_internal->m_type_metadata_lookup.find(type_uuid);
		if (type_metadata_it == m_internal->m_type_metadata_lookup.end())
		{
			// TODO: warning/error?
			return nullptr;
		}

		const TypeMetadata& type_metadata = type_metadata_it->second;
		auto metadata_string_it = type_metadata.metadata_lookup.find(std::string(key));
		if (metadata_string_it != type_metadata.metadata_lookup.end())
		{
			return metadata_string_it->second.c_str();
		}

		return nullptr;
	}

	void MetadataRegistry::set_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key, const char* value)
	{
		VADON_ASSERT(type_uuid.is_valid(), "Invalid type UUID!");
		VADON_ASSERT(property_uuid.is_valid() == true, "Invalid property UUID!");
		VADON_ASSERT(key != nullptr, "Invalid metadata key!");

		auto type_metadata_it = m_internal->m_type_metadata_lookup.find(type_uuid);
		if (type_metadata_it == m_internal->m_type_metadata_lookup.end())
		{
			type_metadata_it = m_internal->m_type_metadata_lookup.insert(std::make_pair(type_uuid, TypeMetadata())).first;
		}

		TypeMetadata& type_metadata = type_metadata_it->second;
		auto property_it = type_metadata.property_lookup.find(property_uuid);
		if (property_it == type_metadata.property_lookup.end())
		{
			property_it = type_metadata.property_lookup.insert(std::make_pair(property_uuid, PropertyMetadata())).first;
		}

		PropertyMetadata& property_metadata = property_it->second;
		property_metadata.metadata_lookup[std::string(key)] = std::string(value);
	}

	const char* MetadataRegistry::get_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key) const
	{
		VADON_ASSERT(type_uuid.is_valid(), "Invalid type ID!");
		VADON_ASSERT(property_uuid.is_valid() == true, "Invalid property UUID!");
		VADON_ASSERT(key != nullptr, "Invalid metadata key!");

		auto type_metadata_it = m_internal->m_type_metadata_lookup.find(type_uuid);
		if (type_metadata_it == m_internal->m_type_metadata_lookup.end())
		{
			VADON_ERROR("Type not registered!");
			return nullptr;
		}

		const TypeMetadata& type_metadata = type_metadata_it->second;
		auto property_it = type_metadata.property_lookup.find(property_uuid);
		if (property_it == type_metadata.property_lookup.end())
		{
			VADON_ERROR("Property not registered!");
			return nullptr;
		}

		const PropertyMetadata& property_metadata = property_it->second;
		auto metadata_string_it = property_metadata.metadata_lookup.find(std::string(key));
		if (metadata_string_it != property_metadata.metadata_lookup.end())
		{
			return metadata_string_it->second.c_str();
		}

		return nullptr;
	}

	MetadataRegistry::MetadataRegistry()
		: m_internal(std::make_unique<Internal>())
	{

	}
}