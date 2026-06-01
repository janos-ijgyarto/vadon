#ifndef VADONEDITOR_CORE_DATA_OBJECT_HPP
#define VADONEDITOR_CORE_DATA_OBJECT_HPP
#include <QMap>
#include <QUuid>
#include <QVariant>
namespace VadonEditor::Core
{
	class Application;

	struct TypeData;

	using PropertyID = QUuid;

	class DataObject
	{
	public:
		DataObject(Application& application);

		bool initialize(const QUuid& type_id);

		const QUuid& get_type_id() const { return m_type_id; }

		QVariant get_property(const PropertyID& property_id) const;
		void set_property(const PropertyID& property_id, const QVariant& value);

		bool serialize(QJsonObject& root_obj) const;
		bool deserialize(const QJsonObject& root_obj);

		bool serialize_properties(QJsonObject& properties_obj) const;
		bool deserialize_properties(const QJsonObject& properties_obj);

		const QVariantMap& get_property_map() const { return m_properties; }
		void load_properties(const QVariantMap& properties);

		static QUuid get_object_type_uuid();
		static QUuid get_type_property_uuid();
		static QUuid get_properties_property_uuid();
	private:
		bool internal_initialize();

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