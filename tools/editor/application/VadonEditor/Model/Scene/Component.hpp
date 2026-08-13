#ifndef VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#define VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#include <VadonEditor/Core/Data/Object.hpp>
#include <QObject>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class Scene;
	class Entity;

	using ComponentID = QUuid;
	class Component : public QObject
	{
		Q_OBJECT
	public:
		Component(Core::Application& application);

		bool initialize(const QUuid& type_id);

		bool import_data(const QVariantMap& data_map) { return m_data.import_data(data_map); }
		QVariantMap export_data() const { return m_data.export_data(); }

		void import_properties(const QVariantMap& properties_map) { m_data.load_properties(properties_map); }

		Core::Application& get_application() { return m_application; }

		const QUuid& get_type_id() const { return m_data.get_type_id(); }

		QVariant get_property(const Core::PropertyID& property_id) const { return m_data.get_property(property_id); }
		void set_property(const Core::PropertyID& property_id, const QVariant& value) { m_data.set_property(property_id, value); emit(property_edited(get_type_id(), property_id)); }

		bool serialize_property(const Core::PropertyID& property_id, QJsonObject& json_obj, bool labeled) const { return m_data.serialize_property_data(property_id, json_obj, labeled); }

		static bool is_type_component(Core::Application& application, const QUuid& type_id);
	signals:
		void property_edited(const QUuid& component_id, const QUuid& property_id);
	private:
		Core::Application& m_application;
		Core::DataObject m_data;

		friend Entity;
	};
}
#endif