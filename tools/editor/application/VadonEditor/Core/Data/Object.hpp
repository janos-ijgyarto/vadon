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

		bool init_type(const QUuid& type_id);
		bool default_initialize(const QUuid& type_id);

		bool is_valid() const { return m_type_id.isNull() == false; }

		bool import_data(const QVariantMap& data_map);
		QVariantMap export_data() const;

		const QUuid& get_type_id() const { return m_type_id; }

		QVariant get_property(const PropertyID& property_id) const;
		void set_property(const PropertyID& property_id, const QVariant& value);

		bool serialize(QJsonObject& root_obj, bool labeled) const;
		bool deserialize(const QJsonObject& root_obj);

		bool serialize_properties(QJsonObject& properties_obj, bool labeled) const;
		bool deserialize_properties(const QJsonObject& properties_obj);

		bool serialize_property_data(const QUuid& property_id, QJsonObject& property_obj, bool labeled) const;

		const QVariantMap& get_property_map() const { return m_properties; }
		void load_properties(const QVariantMap& properties);

		static QUuid get_object_type_uuid();
		static QUuid get_type_property_uuid();
		static QUuid get_properties_property_uuid();

		static QUuid deserialize_object_type(const QJsonObject& object);
	private:
		bool internal_serialize_property_data(QJsonObject& json_obj, const PropertyData& type_property_data, const QVariant& property_value, bool labeled) const;

		void internal_set_property(const PropertyID& property_id, const QVariant& value, bool ignore_deprecated);

		Application& m_application;

		QUuid m_type_id;
		QVariantMap m_properties;
	};
}
#endif