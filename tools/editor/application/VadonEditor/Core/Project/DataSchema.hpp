#ifndef VADONEDITOR_CORE_PROJECT_DATASCHEMA_HPP
#define VADONEDITOR_CORE_PROJECT_DATASCHEMA_HPP
#include <Vadon/Foundation/TypeInfo/MetadataRegistry.hpp>
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
#include <Vadon/Foundation/TypeInfo/Property.hpp>

#include <QByteArray>
#include <QHash>
#include <QUuid>
namespace VadonEditor::Core
{
	struct PropertyData
	{
		::Vadon::Foundation::Property info;
		QHash<QString, QByteArray> metadata;
	};

	struct TypeData
	{
		QHash<QString, QByteArray> metadata;
		QHash<QUuid, PropertyData> properties;
		QList<::Vadon::Foundation::UUID> property_list;
		::Vadon::Foundation::TypeInfo info;
	};

	class DataSchema : public ::Vadon::Foundation::TypeMetadataRegistry
	{
	public:
		bool is_valid() const { return m_types.isEmpty() == false; }

		void register_type(const ::Vadon::Foundation::TypeInfo& type_info) override;

		size_t get_registered_type_count() const override { return m_type_list.size(); }
		::Vadon::Foundation::UUID get_type_uuid(size_t index) const override { return m_type_list[index]; }

		::Vadon::Foundation::TypeInfo get_type_info(const ::Vadon::Foundation::UUID& type_uuid) const override;
		::Vadon::Foundation::UUID get_type_property_uuid(const ::Vadon::Foundation::UUID& type_uuid, size_t property_index) const override;

		void set_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key, const char* value) override;
		const char* get_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key) const override;

		void register_property(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::Property& property) override;

		void set_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key, const char* value) override;
		const char* get_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key) const override;

		bool save_schema(const QString& schema_file_path);
		bool load_schema(const QString& schema_file_path);
	private:
		QHash<QUuid, TypeData> m_types;
		QList<::Vadon::Foundation::UUID> m_type_list; // NOTE: this is used for queries
	};
}
#endif