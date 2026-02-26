#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Model/Resource/Resource.hpp>

#include <QJsonDocument>
#include <QJsonObject>

namespace
{
	struct ResourceFileSerializer
	{
		QFile resource_file;
		QByteArray file_data;
		QJsonDocument json_document;

		ResourceFileSerializer(const QString& path)
			: resource_file(path)
		{

		}

		bool load_resource()
		{
			if (resource_file.open(QIODevice::ReadOnly) == false)
			{
				qCritical("Failed to open resource file!");
				return false;
			}

			file_data = resource_file.readAll();

			// TODO: check error 
			json_document = QJsonDocument::fromJson(file_data);
			if (json_document.isNull() == true)
			{
				qCritical("Resource file is not valid JSON.");
				return false;
			}

			return true;
		}
	};
}

namespace VadonEditor::Model
{
	void ResourceSystem::register_resource_init_data(const QUuid& type_id, const QUuid& data_id)
	{
		if (m_resource_init_data_lookup.find(type_id) != m_resource_init_data_lookup.end())
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::register_resource_data", "Resource data already registered!");
			return;
		}

		m_resource_init_data_lookup.insert(type_id, data_id);
	}

	QUuid ResourceSystem::get_resource_init_data(const QUuid& type_id) const
	{
		auto init_data_it = m_resource_init_data_lookup.find(type_id);
		if (init_data_it == m_resource_init_data_lookup.end())
		{
			return QUuid();
		}

		return init_data_it.value();
	}

	ResourceInfo ResourceSystem::parse_resource_info(const QByteArray& file_data) const
	{
		// TODO: check error 
		QJsonDocument json_document = QJsonDocument::fromJson(file_data);
		if (json_document.isNull() == true)
		{
			qCritical("Resource file is not valid JSON.");
			return ResourceInfo{};
		}
		const QJsonObject& root_object = json_document.object();
		
		ResourceInfo resource_info;
		if (internal_parse_resource_info(resource_info, root_object) == false)
		{
			return ResourceInfo{};
		}

		return resource_info;
	}

	Resource* ResourceSystem::get_resource(const ResourceID& resource_id)
	{
		auto resource_it = m_resource_lookup.find(resource_id);
		if (resource_it == m_resource_lookup.end())
		{
			// Resource not yet loaded, get the file data
			QByteArray resource_file_data;
			if (m_application.get_asset_manager().load_asset_data(resource_id, resource_file_data) == false)
			{
				Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::get_resource", "Unable to load resource file data!");
				return nullptr;
			}

			// TODO: check error 
			QJsonDocument json_document = QJsonDocument::fromJson(resource_file_data);
			if (json_document.isNull() == true)
			{
				Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::get_resource", "Resource file is not valid JSON!");
				return nullptr;
			}
			
			// Parse resource info
			// FIXME: have a LUT of imported resources so we don't have to do this more than once!
			const QJsonObject& root_object = json_document.object();

			ResourceInfo resource_info;
			if (internal_parse_resource_info(resource_info, root_object) == false)
			{
				Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::get_resource", "Invalid resource info!");
				return nullptr;
			}

			Resource* new_resource = new Resource(m_application);
			new_resource->m_info = resource_info;

			if (new_resource->initialize() == false)
			{
				Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::get_resource", "Failed to initialize resource!");
				delete new_resource;
				return nullptr;
			}

			resource_it = m_resource_lookup.insert(resource_info.id, new_resource);
		}

		return resource_it.value();
	}

	QString ResourceSystem::get_imported_file_path(const QFileInfo& resource_file_info)
	{
		// Assume file info is for the .vdimport file
		// The file should be the same path but with the final suffix removed
		return QString("%1/%2").arg(resource_file_info.absolutePath()).arg(resource_file_info.completeBaseName());
	}

	ResourceSystem::ResourceSystem(Core::Application& application)
		: m_application(application)
	{

	}

	bool ResourceSystem::initialize()
	{
		return true;
	}
	
	bool ResourceSystem::internal_parse_resource_info(ResourceInfo& info, const QJsonObject& root_object) const
	{
		if (info.load(root_object) == false)
		{
			qCritical("Invalid resource file data!");
			return false;
		}

		const Core::DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		if (data_schema.is_base_of(Utilities::base64_string_to_vadon_uuid(::Vadon::Foundation::ResourceSchema::c_type_uuid.string), Utilities::qt_uuid_to_vadon_uuid(info.type)) == false)
		{
			qCritical("Content type is not a subclass of Resource!");
			return false;
		}

		return true;
	}
}