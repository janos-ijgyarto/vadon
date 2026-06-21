#ifndef VADONEDITOR_CORE_DATA_SCHEMA_HPP
#define VADONEDITOR_CORE_DATA_SCHEMA_HPP
#include <VadonEditor/Core/Data/Type.hpp>
#include <Vadon/Foundation/TypeInfo/MetadataRegistry.hpp>

#include <QByteArray>
#include <QStandardItemModel>

namespace VadonEditor::Core
{
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

			bool is_base_of(const QUuid& base_uuid, const QUuid& derived_uuid) const;
		private:
			void process_metadata();

			QHash<QUuid, TypeData> m_types;
			QList<::Vadon::Foundation::UUID> m_type_list; // NOTE: this is used for queries

			friend DataSchema;
		};

		bool is_valid() const { return m_registry.get_registered_type_count() > 0; }
		
		TypeMetadataRegistry& get_registry() { return m_registry; }
		const TypeMetadataRegistry& get_registry() const { return m_registry; }

		const TypeData* find_type_data(const QUuid& type_uuid) const;
		const PropertyData* find_type_property_data(const QUuid& type_uuid, const QUuid& property_uuid) const;

		bool save_schema(const QString& schema_file_path);
		bool load_schema(const QString& schema_file_path);

		const QStandardItemModel& get_qt_model() const { return m_qt_model; }
		QModelIndex find_type_index(const QUuid& type_uuid) const;

		bool is_base_of(const QUuid& base_uuid, const QUuid& derived_uuid) const { return m_registry.is_base_of(base_uuid, derived_uuid); }

		::Vadon::Foundation::BaseType get_underlying_base_type(const QUuid& type_uuid) const;
	private:
		void generate_qt_model();

		TypeMetadataRegistry m_registry;
		QStandardItemModel m_qt_model;
	};
}
#endif