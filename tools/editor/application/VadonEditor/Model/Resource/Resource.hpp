#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCE_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCE_HPP
#include <VadonEditor/Core/Data/Object.hpp>
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

		bool is_valid() const { return (Utilities::is_uuid_valid(id) == true) && (Utilities::is_uuid_valid(type) == true); }
	};

	class Resource
	{
	public:
		~Resource();

		Core::Application& get_application() { return m_application; }
		const ResourceInfo& get_info() const { return m_info; }
		bool is_pending_remove() const { return m_pending_remove; }
		bool is_modified() const { return m_modified; }

		void notify_modifed();

		QVariant get_property(const PropertyID& property_id) const { return m_data.get_property(property_id); }
		QVariant get_property_default_value(const PropertyID& property_id) const { return m_data.get_property_default_value(property_id); }
		bool has_property(const PropertyID& property_id) const { return m_data.has_property(property_id); }
		void set_property(const PropertyID& property_id, const QVariant& value);

		const Resource* get_owner() const { return m_owner; }
		bool is_embedded() const { return m_owner != nullptr; }

		Resource* create_embedded_resource(const QUuid& type);

		void open();

		static bool is_resource_base_of_type(Core::Application& application, const QUuid& type_id);
		static bool is_imported_file_base_of_type(Core::Application& application, const QUuid& type_id);

		static QUuid get_base_resource_type();
		static QUuid get_imported_file_resource_type();
	private:
		Resource(Core::Application& application);

		bool initialize();

		bool internal_save(QJsonObject& root_obj, bool labeled) const;
		bool internal_load(const QJsonObject& root_obj);

		void message_resource_created() const;
		void message_resource_asset_created() const;
		void message_resource_loaded(bool reload) const;

		void clear_modified();
		void update_asset_state();

		Core::Application& m_application;

		ResourceInfo m_info;

		Core::DataObject m_data;

		Resource* m_owner; // NOTE: used by embedded resources
		QHash<ResourceID, Resource*> m_embedded_resources;

		bool m_pending_remove; // FIXME: nicer way to do this?
		bool m_modified; // TODO: create more advanced system of "versioning" for changes (undo/redo, etc.)

		friend class ResourceSystem;
	};
}
#endif