#ifndef VADONEDITOR_CORE_TYPEINFO_METADATAREGISTRY_HPP
#define VADONEDITOR_CORE_TYPEINFO_METADATAREGISTRY_HPP
#include <VadonEditor/Core/Editor.hpp>

#include <Vadon/Utilities/TypeInfo/Registry.hpp>
#include <Vadon/Foundation/TypeInfo/MetadataRegistry.hpp>
namespace VadonEditor::Core
{
	class MetadataRegistry : public ::Vadon::Foundation::TypeMetadataRegistry
	{
	public:
		~MetadataRegistry();

		// Metadata utilities
		// FIXME: instead of using string keys, have a more robust system?
		template<typename T>
		void set_type_metadata(std::string_view key, std::string_view value)
		{
			set_type_metadata(Vadon::Utilities::TypeRegistry::get_type_id<T>(), key, value);
		}

		template<typename T>
		std::string get_type_metadata(std::string_view key) const
		{
			return get_type_metadata(Vadon::Utilities::TypeRegistry::get_type_id<T>(), key);
		}

		template<typename T>
		void set_property_metadata(const Vadon::Utilities::PropertyUUID& property_uuid, std::string_view key, std::string_view value)
		{
			set_property_metadata(Vadon::Utilities::TypeRegistry::get_type_id<T>(), property_uuid, key, value);
		}

		size_t get_registered_type_count() const override;
		::Vadon::Foundation::UUID get_type_uuid(size_t index) const override;

		size_t get_type_property_count(const ::Vadon::Foundation::UUID& type_uuid) const override;
		::Vadon::Foundation::UUID get_type_property_uuid(const ::Vadon::Foundation::UUID& type_uuid, size_t property_index) const override;

		void set_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key, const char* value) override;
		const char* get_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key) const override;

		void set_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key, const char* value) override;
		const char* get_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key) const override;
	private:
		MetadataRegistry();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend class Editor;
	};
}
#endif