#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Model/Resource/Resource.hpp>

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{


	bool is_resource_base_of_type(VadonEditor::Core::Application& application, const QUuid& type_id)
	{
		const QUuid resource_type_uuid = VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ResourceSchema::c_type_uuid);
		return application.get_project_manager().get_project_data_schema().is_base_of(VadonEditor::Utilities::qt_uuid_to_vadon_uuid(resource_type_uuid), VadonEditor::Utilities::qt_uuid_to_vadon_uuid(type_id));
	}
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

	Resource* ResourceSystem::create_resource(const QUuid& type_id)
	{
		if (is_resource_base_of_type(m_application, type_id) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::create_resource", "Invalid type UUID");
			return nullptr;
		}

		ResourceInfo new_resource_info;
		new_resource_info.id = QUuid::createUuid();
		new_resource_info.type = type_id;

		return internal_create_new_resource(new_resource_info);
	}

	Resource* ResourceSystem::get_resource(const ResourceID& resource_id)
	{
		auto resource_it = m_resource_lookup.find(resource_id);
		if (resource_it != m_resource_lookup.end())
		{
			return resource_it.value();
		}

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

		Resource* new_resource = internal_create_new_resource(resource_info);
		if (new_resource->internal_load(root_object) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::get_resource", "Failed to load resource data!");
			return nullptr;
		}

		return new_resource;
	}

	bool ResourceSystem::save_resource(Resource* resource)
	{
		QJsonObject root_object;
		if (resource->internal_save(root_object) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::save_resource", "Failed to save resource data!");
			return false;
		}

		if (m_application.get_asset_manager().save_asset_data(resource->get_info().id, QJsonDocument(root_object).toJson()) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::save_resource", "Failed to save resource to file!");
			return false;
		}

		return true;
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
		QObject::connect(&m_application.get_asset_manager(), &Core::AssetManager::asset_opened,
			[this](const QModelIndex& index)
			{
				const Core::AssetInfo asset_info = m_application.get_asset_manager().get_asset_info(index);
				if (asset_info.type == Core::AssetType::RESOURCE)
				{
					// TODO: open resource editor!
					qDebug() << "Open requested for resource asset" << asset_info.path;
				}
			}
		);
		return true;
	}

	Resource* ResourceSystem::internal_create_new_resource(const ResourceInfo& info)
	{
		Q_ASSERT_X(m_resource_lookup.find(info.id) == m_resource_lookup.end(), "VadonEditor::Model::ResourceSystem::internal_create_new_resource", "Resource already added!");

		Resource* new_resource = new Resource(m_application);
		new_resource->m_info = info;

		if (new_resource->initialize() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::internal_create_new_resource", "Failed to initialize resource!");
			delete new_resource;
			return nullptr;
		}

		// Add to lookup
		m_resource_lookup.insert(info.id, new_resource);
		return new_resource;
	}
	
	bool ResourceSystem::internal_parse_resource_info(ResourceInfo& info, const QJsonObject& root_object) const
	{
		if (info.load(root_object) == false)
		{
			qCritical("Invalid resource file data!");
			return false;
		}

		if (is_resource_base_of_type(m_application, info.type) == false)
		{
			qCritical("Content type is not a subclass of Resource!");
			return false;
		}

		return true;
	}
}