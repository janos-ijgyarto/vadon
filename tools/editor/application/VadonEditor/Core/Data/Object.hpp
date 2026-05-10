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

		QVariant get_property(const PropertyID& property_id) const;
		void set_property(const PropertyID& property_id, const QVariant& value);

		bool serialize(QJsonObject& root_obj) const;
		bool deserialize(const QJsonObject& root_obj);

		bool serialize_properties(QJsonObject& properties_obj) const;
		bool deserialize_properties(const QJsonObject& properties_obj);
	private:
		bool internal_initialize();

		bool serialize_object_to_json(const TypeData* type_data, const QVariant& object_data, QJsonObject& json_object) const;
		bool serialize_array_to_json(const QUuid& array_data_type, const QVariant& array_data, QJsonArray& json_array) const;

		bool deserialize_object_from_json(const TypeData* type_data, const QJsonObject& json_object, QVariant& object_data) const;
		bool deserialize_array_from_json(const QUuid& array_data_type, const QJsonArray& json_array, QVariant& array_data) const;

		Application& m_application;

		QUuid m_type_id;
		QVariantMap m_properties;
	};
}
#endif