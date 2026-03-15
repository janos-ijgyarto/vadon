#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCE_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCE_HPP
#include <QHash>
#include <QUuid>
#include <QVariant>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	using ResourceID = QUuid;
	using TypeID = QUuid;
	using PropertyID = QUuid;

	class Scene;

	struct ResourceInfo
	{
		QUuid id;
		QUuid type;

		bool load(const QJsonObject& root_obj);
	};

	class Resource
	{
	public:
		~Resource();

		const ResourceInfo& get_info() const { return m_info; }

		QVariant get_property(const PropertyID& property_id) const;
		void set_property(const PropertyID& property_id, const QVariant& value);

		QVariant get_data(const QUuid& data_id) const;
		void set_data(const QUuid& data_id, const QVariant& value);
	private:
		Resource(Core::Application& application);

		bool initialize();

		bool internal_save(QJsonObject& root_obj) const;
		bool internal_load(const QJsonObject& root_obj);

		Core::Application& m_application;

		ResourceInfo m_info;

		// FIXME: could use an array and get the offsets from a "schema"
		QHash<PropertyID, QVariant> m_properties;
		QHash<QUuid, QVariant> m_data;

		Resource* m_owner; // NOTE: used by embedded resources
		QHash<ResourceID, Resource*> m_embedded_resources;

		friend class ResourceSystem;
	};
}
#endif