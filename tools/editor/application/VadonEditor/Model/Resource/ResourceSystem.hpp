#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#include <VadonEditor/Core/Asset/Asset.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>
#include <QHash>
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

		ResourceInfo parse_resource_info(const QByteArray& file_data) const;

		// TODO: create central UUID system to keep track of all UUIDs (to ensure uniqueness)
		// Have it store metadata to make it easier to retrace where said UUID is used
		Resource* find_resource(const ResourceID& resource_id) const;
		int find_resource_asset_id(const ResourceID& resource_id) const;
		ResourceInfo resource_info_by_asset_id(int asset_id) const;

		Resource* create_resource(const QUuid& type_id);
		Resource* get_resource(const ResourceID& resource_id);
		void remove_resource(Resource* resource);

		int create_resource_asset(const ResourceID& resource_id, const QString& path);
		QList<int> get_resource_asset_list() const;
		bool save_resource(const Resource* resource);
		bool reload_resource(Resource* resource);

		Core::AssetType get_asset_type_for_resource_type(const QUuid& type_id) const;
	private:
		ResourceSystem(Core::Application& application);

		bool initialize();
		void project_loaded();
		void shutdown();

		Resource* internal_create_new_resource(const ResourceInfo& info);
		void internal_add_new_resource(Resource* resource);

		bool internal_parse_resource_info(ResourceInfo& info, const QJsonObject& root_object) const;
		bool internal_add_resource_asset(const ResourceInfo& info, int asset_id);

		bool internal_load_resource(Resource* resource, int asset_id);

		Core::Application& m_application;
		QHash<ResourceID, Resource*> m_resource_lookup;

		QHash<QUuid, QUuid> m_resource_init_data_lookup;

		QHash<QUuid, int> m_resource_asset_lookup;
		QHash<int, ResourceInfo> m_resource_asset_reverse_lookup;

		friend class ModelSystem;
	};
}
#endif