#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#include <VadonEditor/Model/Resource/Resource.hpp>
#include <QHash>
class QFileInfo;
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class ResourceSystem
	{
	public:
		~ResourceSystem();

		// TODO: create central UUID system to keep track of all UUIDs (to ensure uniqueness)
		// Have it store metadata to make it easier to retrace where said UUID is used
		void register_resource_init_data(const QUuid& type_id, const QUuid& data_id);
		QUuid get_resource_init_data(const QUuid& type_id) const;

		ResourceInfo parse_resource_info(const QByteArray& file_data) const;

		Resource* find_resource(const ResourceID& resource_id) const;
		int find_resource_asset_id(const ResourceID& resource_id) const;
		ResourceInfo resource_info_by_asset_id(int asset_id) const;

		Resource* create_resource(const QUuid& type_id);
		Resource* get_resource(const ResourceID& resource_id);
		void remove_resource(Resource* resource);

		int create_resource_asset(const ResourceID& resource_id, const QString& path);
		bool save_resource(const Resource* resource);
	private:
		ResourceSystem(Core::Application& application);

		bool initialize();
		void project_loaded();

		Resource* internal_create_new_resource(const ResourceInfo& info);
		bool internal_parse_resource_info(ResourceInfo& info, const QJsonObject& root_object) const;

		Core::Application& m_application;
		QHash<ResourceID, Resource*> m_resource_lookup;

		QHash<QUuid, QUuid> m_resource_init_data_lookup;

		QHash<QUuid, int> m_resource_asset_lookup;
		QHash<int, ResourceInfo> m_resource_asset_reverse_lookup;

		friend class ModelSystem;
	};
}
#endif