#ifndef VADONEDITOR_CORE_DATA_OBJECT_HPP
#define VADONEDITOR_CORE_DATA_OBJECT_HPP
#include <VadonEditor/Utilities/UUID.hpp>
#include <QMap>
#include <QVariant>
#include <QSpan>
namespace VadonEditor::Core
{
	class Application;

	struct TypeData;
	struct PropertyData;

	struct PropertyPathElement
	{
		qsizetype index = 0;
		QUuid uuid;

		bool is_object_key() const { return Utilities::is_uuid_valid(uuid); }
		bool is_array_index() const { return is_object_key() == false; }
	};

	using PropertyPath = QSpan<PropertyPathElement>;

	using PropertyID = QUuid;

	class DataObject
	{
	public:
		DataObject(Application& application);

		bool init_type(const QUuid& type_id);
		bool default_initialize(const QUuid& type_id);

		bool is_valid() const { return Utilities::is_uuid_valid(m_type_id); }

		bool import_data(const QVariantMap& data_map);
		QVariantMap export_data() const;

		const QUuid& get_type_id() const { return m_type_id; }

		QVariant get_property(const PropertyID& property_id) const;
		void set_property(const PropertyID& property_id, const QVariant& value);

		QVariant get_property(PropertyPath property_path) const;
		void set_property(PropertyPath property_path, const QVariant& value);

		// NOTE: these assume container properties!
		void add_property(PropertyPath property_path, const QVariant& value);
		void remove_property(PropertyPath property_path);

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

		static bool serialize_property_value(Application& application, QJsonObject& data_obj, const PropertyData& property_data, QVariant& property_value);
	private:
		bool internal_serialize_property_data(QJsonObject& json_obj, const PropertyData& type_property_data, const QVariant& property_value, bool labeled) const;

		void internal_set_property(const PropertyID& property_id, const QVariant& value, bool ignore_deprecated);

		Application& m_application;

		QUuid m_type_id;
		QVariantMap m_properties;
	};
}
#endif