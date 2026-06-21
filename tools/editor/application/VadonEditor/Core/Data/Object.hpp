#ifndef VADONEDITOR_CORE_DATA_OBJECT_HPP
#define VADONEDITOR_CORE_DATA_OBJECT_HPP
#include <QMap>
#include <QUuid>
#include <QVariant>
namespace VadonEditor::Core
{
	class Application;

	struct TypeData;
	struct PropertyData;

	using PropertyID = QUuid;

	class DataObject
	{
	public:
		DataObject(Application& application);

		bool is_valid() const { return m_type_id.isNull() == false; }

		bool initialize(const QUuid& type_id);

		bool import_data(const QVariantMap& data_map);
		QVariantMap export_data() const;

		const QUuid& get_type_id() const { return m_type_id; }

		QVariant get_property(const PropertyID& property_id) const;
		void set_property(const PropertyID& property_id, const QVariant& value);

		bool serialize(QJsonObject& root_obj) const;
		bool deserialize(const QJsonObject& root_obj);

		bool serialize_properties(QJsonObject& properties_obj) const;
		bool deserialize_properties(const QJsonObject& properties_obj);

		bool serialize_property_data(const QUuid& property_id, QJsonObject& property_obj) const;

		const QVariantMap& get_property_map() const { return m_properties; }
		void load_properties(const QVariantMap& properties);

		static QUuid get_object_type_uuid();
		static QUuid get_type_property_uuid();
		static QUuid get_properties_property_uuid();
	private:
		bool internal_initialize();

		bool internal_serialize_property_data(const PropertyData& type_property_data, const QVariant& property_value, QJsonObject& json_obj) const;

		bool serialize_generic_object_to_json(const QVariant& object_data, QJsonObject& json_object) const;
		bool deserialize_generic_object_from_json(const QJsonObject& json_object, QVariant& object_data) const;

		bool serialize_typed_object_to_json(const TypeData* type_data, const QVariant& object_data, QJsonObject& json_object) const;
		bool deserialize_typed_object_from_json(const TypeData* type_data, const QJsonObject& json_object, QVariant& object_data) const;
		
		Application& m_application;

		QUuid m_type_id;
		QVariantMap m_properties;
	};
}
#endif