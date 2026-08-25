#include <VadonEditor/Core/TypeInfo/MetadataRegistry.hpp>

#include <Vadon/Utilities/Debugging/Assert.hpp>

namespace
{
	struct PropertyMetadata
	{
		::Vadon::Foundation::Property base_info;
		std::vector<::Vadon::Foundation::UUID> type_list;
		std::unordered_map<std::string, std::string> metadata_lookup;
	};

	struct TypeMetadata
	{
		::Vadon::Foundation::TypeInfo info;
		std::unordered_map<std::string, std::string> metadata_lookup;
		std::unordered_map<Vadon::Utilities::PropertyUUID, PropertyMetadata> property_lookup;
	};

	struct TypePropertyListCache
	{
		Vadon::Utilities::TypeUUID cached_type;
		std::vector<Vadon::Utilities::PropertyUUID> properties;

		void update(const ::Vadon::Foundation::UUID& type_uuid)
		{
			if (type_uuid != cached_type)
			{
				properties.clear();

				const Vadon::Utilities::PropertyInfoList type_properties = Vadon::Utilities::TypeRegistry::get_type_properties(Vadon::Utilities::TypeRegistry::get_type_id(type_uuid));
				for (const Vadon::Utilities::PropertyInfo& current_property_info : type_properties)
				{
					properties.push_back(current_property_info.base_info.id);
				}

				cached_type = type_uuid;
			}
		}
	};

	TypePropertyListCache s_property_list_cache;
}

namespace VadonEditor::Core
{
	struct MetadataRegistry::Internal
	{
		std::vector<Vadon::Utilities::TypeUUID> m_registered_types;

		std::unordered_map<Vadon::Utilities::TypeUUID, TypeMetadata> m_type_metadata_lookup;
	};

	MetadataRegistry::~MetadataRegistry() = default;

	void MetadataRegistry::register_type(const::Vadon::Foundation::TypeInfo& type_info)
	{
		VADON_ASSERT(type_info.id.is_valid() == true, "Type not registered!");

		if (m_internal->m_type_metadata_lookup.find(type_info.id) != m_internal->m_type_metadata_lookup.end())
		{
			// TODO: log warning that type was already registered!
			return;
		}

		TypeMetadata metadata;
		metadata.info = type_info;

		m_internal->m_type_metadata_lookup.insert(std::make_pair(type_info.id, metadata));
	}

	size_t MetadataRegistry::get_registered_type_count() const
	{
		return m_internal->m_registered_types.size();
	}

	::Vadon::Foundation::UUID MetadataRegistry::get_type_uuid(size_t index) const
	{
		return m_internal->m_registered_types[index];
	}

	::Vadon::Foundation::TypeInfo MetadataRegistry::get_type_info(const::Vadon::Foundation::UUID& type_uuid) const
	{
		VADON_ASSERT(type_uuid.is_valid(), "Invalid type UUID!");

		auto type_metadata_it = m_internal->m_type_metadata_lookup.find(type_uuid);
		if (type_metadata_it == m_internal->m_type_metadata_lookup.end())
		{
			return ::Vadon::Foundation::TypeInfo();
		}

		return type_metadata_it->second.info;
	}

	::Vadon::Foundation::UUID MetadataRegistry::get_type_property_uuid(const ::Vadon::Foundation::UUID& type_uuid, size_t property_index) const
	{
		s_property_list_cache.update(type_uuid);
		return s_property_list_cache.properties[property_index];
	}

	void MetadataRegistry::set_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key, const char* value)
	{
		VADON_ASSERT(type_uuid.is_valid(), "Invalid type UUID!");
		VADON_ASSERT(key != nullptr, "Invalid key!");
		VADON_ASSERT(std::string_view(key).empty() == false, "Invalid key!");

		auto type_metadata_it = m_internal->m_type_metadata_lookup.find(type_uuid);
		if (type_metadata_it == m_internal->m_type_metadata_lookup.end())
		{
			VADON_ERROR("Type not registered!");
			return;
		}

		TypeMetadata& type_metadata = type_metadata_it->second;
		type_metadata.metadata_lookup[key] = value;
	}

	const char* MetadataRegistry::get_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key) const
	{
		VADON_ASSERT(type_uuid.is_valid(), "Invalid type UUID!");
		VADON_ASSERT(key != nullptr, "Invalid key!");
		VADON_ASSERT(std::string_view(key).empty() == false, "Invalid key!");

		auto type_metadata_it = m_internal->m_type_metadata_lookup.find(type_uuid);
		if (type_metadata_it == m_internal->m_type_metadata_lookup.end())
		{
			VADON_ERROR("Type not registered!");
			// TODO: log warning/error?
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

	void MetadataRegistry::register_property(const::Vadon::Foundation::UUID& type_uuid, const::Vadon::Foundation::Property& property, const ::Vadon::Foundation::UUID* type_list)
	{
		VADON_ASSERT(type_uuid.is_valid() == true, "Invalid type UUID!");
		VADON_ASSERT(property.is_valid() == true, "Invalid property info!");

		auto type_it = m_internal->m_type_metadata_lookup.find(type_uuid);
		if (type_it == m_internal->m_type_metadata_lookup.end())
		{
			VADON_ERROR("Type not registered!");
			return;
		}

		TypeMetadata& type_metadata = type_it->second;
		auto property_it = type_metadata.property_lookup.find(property.id);
		if (property_it != type_metadata.property_lookup.end())
		{
			// TODO: log warning!
			return;
		}

		PropertyMetadata property_data;
		property_data.base_info = property;
		property_data.type_list = std::vector<::Vadon::Foundation::UUID>(type_list, type_list + property_data.base_info.type_list_length);

		type_metadata.property_lookup.insert(std::make_pair(property.id, property_data));
	}

	::Vadon::Foundation::Property VadonEditor::Core::MetadataRegistry::get_property_info(const ::Vadon::Foundation::UUID& type_uuid, const::Vadon::Foundation::UUID& property_uuid) const
	{
		VADON_ASSERT(type_uuid.is_valid() == true, "Invalid type UUID!");
		VADON_ASSERT(property_uuid.is_valid() == true, "Invalid property UUID!");

		auto type_it = m_internal->m_type_metadata_lookup.find(type_uuid);
		if (type_it == m_internal->m_type_metadata_lookup.end())
		{
			VADON_ERROR("Type not registered!");
			return ::Vadon::Foundation::Property{};
		}

		const TypeMetadata& type_metadata = type_it->second;
		auto property_it = type_metadata.property_lookup.find(property_uuid);
		if (property_it == type_metadata.property_lookup.end())
		{
			VADON_ERROR("Property not registered!");
			return ::Vadon::Foundation::Property{};
		}

		return property_it->second.base_info;
	}

	::Vadon::Foundation::UUID MetadataRegistry::get_property_type_list_entry(const::Vadon::Foundation::UUID& type_uuid, const::Vadon::Foundation::UUID& property_uuid, size_t index) const
	{
		VADON_ASSERT(type_uuid.is_valid() == true, "Invalid type UUID!");
		VADON_ASSERT(property_uuid.is_valid() == true, "Invalid property UUID!");

		auto type_it = m_internal->m_type_metadata_lookup.find(type_uuid);
		if (type_it == m_internal->m_type_metadata_lookup.end())
		{
			VADON_ERROR("Type not registered!");
			return ::Vadon::Foundation::UUID{};
		}

		const TypeMetadata& type_metadata = type_it->second;
		auto property_it = type_metadata.property_lookup.find(property_uuid);
		if (property_it == type_metadata.property_lookup.end())
		{
			VADON_ERROR("Property not registered!");
			return ::Vadon::Foundation::UUID{};
		}

		const PropertyMetadata& property_metadata = property_it->second;
		if (index >= property_metadata.type_list.size())
		{
			VADON_ERROR("Invalid index!");
			return ::Vadon::Foundation::UUID{};
		}

		return property_metadata.type_list[index];
	}

	void MetadataRegistry::set_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key, const char* value)
	{
		VADON_ASSERT(type_uuid.is_valid(), "Invalid type UUID!");
		VADON_ASSERT(property_uuid.is_valid() == true, "Invalid property UUID!");
		VADON_ASSERT(key != nullptr, "Invalid key!");
		VADON_ASSERT(std::string_view(key).empty() == false, "Invalid key!");

		auto type_metadata_it = m_internal->m_type_metadata_lookup.find(type_uuid);
		if (type_metadata_it == m_internal->m_type_metadata_lookup.end())
		{
			VADON_ERROR("Type not registered!");
			return;
		}
		
		TypeMetadata& type_metadata = type_metadata_it->second;
		auto property_it = type_metadata.property_lookup.find(property_uuid);
		if (property_it == type_metadata.property_lookup.end())
		{
			VADON_ERROR("Property not registered!");
			return;
		}

		PropertyMetadata& property_metadata = property_it->second;
		property_metadata.metadata_lookup.insert(std::make_pair(std::string(key), std::string(value)));
	}

	const char* MetadataRegistry::get_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key) const
	{
		VADON_ASSERT(type_uuid.is_valid(), "Invalid type ID!");
		VADON_ASSERT(property_uuid.is_valid() == true, "Invalid property UUID!");
		VADON_ASSERT(key != nullptr, "Invalid key!");
		VADON_ASSERT(std::string_view(key).empty() == false, "Invalid key!");

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
		auto metadata_string_it = property_metadata.metadata_lookup.find(key);
		if (metadata_string_it == property_metadata.metadata_lookup.end())
		{
			return nullptr;
		}

		return metadata_string_it->second.c_str();
	}

	MetadataRegistry::MetadataRegistry()
		: m_internal(std::make_unique<Internal>())
	{

	}

	void MetadataRegistry::initialize()
	{
		m_internal->m_registered_types = Vadon::Utilities::TypeRegistry::get_all_registered_types();
	}
}