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

		bool is_valid() const { return (id.isNull() == false) && (type.isNull() == false); }
	};

	class Resource
	{
	public:
		~Resource();

		Core::Application& get_application() { return m_application; }
		const ResourceInfo& get_info() const { return m_info; }
		bool is_pending_remove() const { return m_pending_remove; }
		bool is_modified() const { return m_modified; }

		void notify_modifed() { m_modified = true; }

		QVariant get_property(const PropertyID& property_id) const;
		void set_property(const PropertyID& property_id, const QVariant& value);

		const Resource* get_owner() const { return m_owner; }

		Resource* create_embedded_resource(const QUuid& type);

		static bool is_resource_base_of_type(Core::Application& application, const QUuid& type_id);
		static bool is_imported_file_base_of_type(Core::Application& application, const QUuid& type_id);

		static QUuid get_base_resource_type();
		static QUuid get_imported_file_resource_type();
	private:
		Resource(Core::Application& application);

		bool initialize();

		bool internal_save(QJsonObject& root_obj) const;
		bool internal_load(const QJsonObject& root_obj);

		void clear_modified() { m_modified = false; }

		Core::Application& m_application;

		ResourceInfo m_info;

		// FIXME: could use an array and get the offsets from a "schema"
		QHash<PropertyID, QVariant> m_properties;
		QHash<QUuid, QVariant> m_data;

		Resource* m_owner; // NOTE: used by embedded resources
		QHash<ResourceID, Resource*> m_embedded_resources;

		bool m_pending_remove; // FIXME: nicer way to do this?
		bool m_modified; // TODO: create more advanced system of "versioning" for changes (undo/redo, etc.)

		friend class ResourceSystem;
	};
}
#endif