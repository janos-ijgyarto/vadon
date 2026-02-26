#ifndef VADONEDITOR_CORE_PROJECT_DATASCHEMA_HPP
#define VADONEDITOR_CORE_PROJECT_DATASCHEMA_HPP
#include <Vadon/Foundation/TypeInfo/Metadata.hpp>
#include <Vadon/Foundation/TypeInfo/MetadataRegistry.hpp>
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
#include <Vadon/Foundation/TypeInfo/Property.hpp>

#include <QByteArray>
#include <QHash>
#include <QStandardItemModel>
#include <QUuid>

namespace VadonEditor::Core
{
	enum class PropertyCategory
	{
		TRIVIAL,
		RESOURCE,
		ARRAY
	};

	struct PropertyData
	{
		::Vadon::Foundation::Property info;
		QHash<QString, QByteArray> metadata;

		QString find_metadata(const char* key) const;
		QString find_metadata(::Vadon::Foundation::CommonPropertyMetadata::Key key) const { return find_metadata(::Vadon::Foundation::CommonPropertyMetadata::key_string(key)); }

		PropertyCategory get_category() const;

		QString get_name() const;
		::Vadon::Foundation::UUID get_data_type() const;
	};

	struct TypeData
	{
		QHash<QString, QByteArray> metadata;
		QHash<QUuid, PropertyData> properties;
		QList<::Vadon::Foundation::UUID> property_list;
		::Vadon::Foundation::TypeInfo info;

		const PropertyData* find_property_data(const ::Vadon::Foundation::UUID& property_uuid) const;

		QString find_metadata(const char* key) const;
		QString find_metadata(::Vadon::Foundation::CommonTypeMetadata::Key key) const { return find_metadata(::Vadon::Foundation::CommonTypeMetadata::key_string(key)); }

		QString get_name() const;
	};

	enum class TypeTreeDataRole
	{
		TYPE_UUID = Qt::ItemDataRole::UserRole + 1
	};

	class DataSchema
	{
	public:
		class TypeMetadataRegistry : public ::Vadon::Foundation::TypeMetadataRegistry
		{
		public:
			void register_type(const ::Vadon::Foundation::TypeInfo& type_info) override;

			size_t get_registered_type_count() const override { return m_type_list.size(); }
			::Vadon::Foundation::UUID get_type_uuid(size_t index) const override { return m_type_list[index]; }

			::Vadon::Foundation::TypeInfo get_type_info(const ::Vadon::Foundation::UUID& type_uuid) const override;
			::Vadon::Foundation::UUID get_type_property_uuid(const ::Vadon::Foundation::UUID& type_uuid, size_t property_index) const override;

			void set_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key, const char* value) override;
			const char* get_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key) const override;

			void register_property(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::Property& property) override;
			::Vadon::Foundation::Property get_property_info(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid) const override;

			void set_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key, const char* value) override;
			const char* get_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key) const override;

			bool is_base_of(const ::Vadon::Foundation::UUID& base_uuid, const ::Vadon::Foundation::UUID& derived_uuid) const;
		private:
			QHash<QUuid, TypeData> m_types;
			QList<::Vadon::Foundation::UUID> m_type_list; // NOTE: this is used for queries

			friend DataSchema;
		};

		bool is_valid() const { return m_registry.get_registered_type_count() > 0; }
		
		TypeMetadataRegistry& get_registry() { return m_registry; }
		const TypeMetadataRegistry& get_registry() const { return m_registry; }

		const TypeData* find_type_data(const ::Vadon::Foundation::UUID& type_uuid) const;

		bool save_schema(const QString& schema_file_path);
		bool load_schema(const QString& schema_file_path);

		static ::Vadon::Foundation::UUID get_base_type_uuid(::Vadon::Foundation::BaseType type);
		static ::Vadon::Foundation::BaseType get_base_type(const ::Vadon::Foundation::UUID& type_uuid);

		const QStandardItemModel& get_qt_model() const { return m_qt_model; }

		bool is_base_of(const ::Vadon::Foundation::UUID& base_uuid, const ::Vadon::Foundation::UUID& derived_uuid) const { return m_registry.is_base_of(base_uuid, derived_uuid); }
	private:
		void generate_qt_model();

		TypeMetadataRegistry m_registry;
		QStandardItemModel m_qt_model;
	};
}
#endif