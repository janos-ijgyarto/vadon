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
		void register_resource_init_data(const QUuid& type_id, const QUuid& data_id);
		QUuid get_resource_init_data(const QUuid& type_id) const;

		ResourceInfo parse_resource_info(const QByteArray& file_data) const;

		Resource* create_resource(const QUuid& type_id);
		Resource* get_resource(const ResourceID& resource_id);

		bool save_resource(Resource* resource);

		static QString get_imported_file_path(const QFileInfo& resource_file_info);
	private:
		ResourceSystem(Core::Application& application);

		bool initialize();

		Resource* internal_create_new_resource(const ResourceInfo& info);
		bool internal_parse_resource_info(ResourceInfo& info, const QJsonObject& root_object) const;

		Core::Application& m_application;
		QHash<ResourceID, Resource*> m_resource_lookup;
		QHash<QUuid, QUuid> m_resource_init_data_lookup;

		friend class ModelSystem;
	};
}
#endif