#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Model/Resource/File.hpp>
#include <Vadon/Foundation/Model/Resource/Resource.hpp>
#include <Vadon/Foundation/Model/Scene/Scene.hpp>

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace VadonEditor::Model
{
	ResourceSystem::~ResourceSystem()
	{
		Q_ASSERT_X(m_resource_lookup.empty() == true, "VadonEditor::Model::ResourceSystem::~ResourceSystem", "System was not shut down properly");
	}

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

		ResourceInfo resource_info;
		if (internal_parse_resource_info(resource_info, json_document.object()) == false)
		{
			return ResourceInfo{};
		}

		return resource_info;
	}

	Resource* ResourceSystem::create_resource(const QUuid& type_id)
	{
		if (Resource::is_resource_base_of_type(m_application, type_id) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::create_resource", "Invalid type UUID");
			return nullptr;
		}

		ResourceInfo new_resource_info;
		new_resource_info.id = QUuid::createUuid();
		new_resource_info.type = type_id;

		return internal_create_new_resource(new_resource_info);
	}

	Resource* ResourceSystem::find_resource(const ResourceID& resource_id) const
	{
		auto resource_it = m_resource_lookup.find(resource_id);
		if (resource_it != m_resource_lookup.end())
		{
			return resource_it.value();
		}

		return nullptr;
	}

	int ResourceSystem::find_resource_asset_id(const ResourceID& resource_id) const
	{
		auto resource_asset_it = m_resource_asset_lookup.find(resource_id);
		if (resource_asset_it != m_resource_asset_lookup.end())
		{
			return resource_asset_it.value();
		}

		return Core::AssetInfo::c_invalid_file_id;
	}

	ResourceInfo ResourceSystem::resource_info_by_asset_id(int asset_id) const
	{
		auto resource_info_it = m_resource_asset_reverse_lookup.find(asset_id);
		if (resource_info_it != m_resource_asset_reverse_lookup.end())
		{
			return resource_info_it.value();
		}

		return ResourceInfo();
	}

	Resource* ResourceSystem::get_resource(const ResourceID& resource_id)
	{
		Resource* resource = find_resource(resource_id);
		if(resource != nullptr)
		{
			return resource;
		}

		// Resource not yet loaded, get the file data
		const int resource_asset_id = find_resource_asset_id(resource_id);
		if (resource_asset_id == Core::AssetInfo::c_invalid_file_id)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::get_resource", "No asset file associated with this resource!");
			return nullptr;
		}

		QByteArray resource_file_data;
		if (m_application.get_asset_manager().load_asset_data(resource_asset_id, resource_file_data) == false)
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

	void ResourceSystem::remove_resource(Resource* resource)
	{
		if (resource->m_pending_remove == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::remove_resource", "Cannot remove active resource!");
			return;
		}

		auto resource_it = m_resource_lookup.find(resource->get_info().id);
		if (resource_it == m_resource_lookup.end())
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::remove_resource", "Resource was not in lookup!");
			return;
		}

		m_resource_lookup.erase(resource_it);
	}

	int ResourceSystem::create_resource_asset(const ResourceID& resource_id, const QString& path)
	{
		Resource* resource = find_resource(resource_id);
		if (resource == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::create_resource_asset", "Cannot find resource!");
			return Core::AssetInfo::c_invalid_file_id;
		}

		int asset_id = find_resource_asset_id(resource_id);
		if (asset_id != Core::AssetInfo::c_invalid_file_id)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::create_resource_asset", "Resource asset already exists!");
			return asset_id;
		}

		Core::AssetManager& asset_manager = m_application.get_asset_manager();

		Core::AssetInfo resource_asset_info;
		resource_asset_info.path = Core::AssetInfo::get_file_path(path, Core::AssetType::RESOURCE);
		resource_asset_info.type = Core::AssetType::RESOURCE;

		QModelIndex asset_index = asset_manager.create_asset(resource_asset_info);
		if (asset_index.isValid() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::create_resource_asset", "Cannot create asset!");
			return Core::AssetInfo::c_invalid_file_id;
		}

		asset_id = asset_manager.get_asset_info(asset_index).id;
		
		if (internal_add_resource_asset(resource->get_info(), asset_id) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::create_resource_asset", "Failed to register resource asset!");
			return Core::AssetInfo::c_invalid_file_id;
		}

		// Save the resource for the first time
		if (save_resource(resource) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::create_resource_asset", "Failed to save resource to file!");
		}

		return asset_id;
	}
	
	QList<int> ResourceSystem::get_resource_asset_list() const
	{
		return m_resource_asset_reverse_lookup.keys();
	}

	bool ResourceSystem::save_resource(const Resource* resource)
	{
		if (resource->get_owner())
		{
			return save_resource(resource->get_owner());
		}

		const int asset_id = find_resource_asset_id(resource->get_info().id);
		if (asset_id == Core::AssetInfo::c_invalid_file_id)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::save_resource", "Resource does not have asset!");
			return false;
		}

		QJsonObject root_object;
		if (resource->internal_save(root_object) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::save_resource", "Failed to save resource data!");
			return false;
		}

		if (m_application.get_asset_manager().save_asset_data(asset_id, QJsonDocument(root_object).toJson()) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::save_resource", "Failed to save resource to file!");
			return false;
		}

		return true;
	}

	Core::AssetType ResourceSystem::get_asset_type_for_resource_type(const QUuid& type_id) const
	{
		if (Scene::is_scene_base_of_type(m_application, type_id) == true)
		{
			return Core::AssetType::SCENE;
		}

		if (Resource::is_imported_file_base_of_type(m_application, type_id) == true)
		{
			return Core::AssetType::IMPORTED_FILE;
		}

		if (Resource::is_resource_base_of_type(m_application, type_id) == true)
		{
			return Core::AssetType::RESOURCE;
		}

		return Core::AssetType::NONE;
	}

	ResourceSystem::ResourceSystem(Core::Application& application)
		: m_application(application)
	{

	}

	bool ResourceSystem::initialize()
	{
		return true;
	}

	void ResourceSystem::project_loaded()
	{
		Core::AssetManager& asset_manager = m_application.get_asset_manager();

		QStandardItemModel& model = asset_manager.get_model();
		QStandardItem* root_item = model.invisibleRootItem();

		auto process_assets_recursive = [&](QStandardItem* current_root) -> void {
			auto do_process_assets_recursive = [&](const auto& self, QStandardItem* current_root) -> void {
				for (int child_index = 0; child_index < current_root->rowCount(); ++child_index)
				{
					QStandardItem* current_child = current_root->child(child_index);
					const Core::AssetInfo asset_info = asset_manager.get_asset_info(current_child->index());

					switch (asset_info.type)
					{
					case Core::AssetType::RESOURCE:
					case Core::AssetType::SCENE:
					case Core::AssetType::IMPORTED_FILE:
					{
						QByteArray asset_data;
						if (asset_manager.load_asset_data(asset_info.id, asset_data) == false)
						{
							qCritical() << "Failed to load resource asset data!";
							return;
						}

						const ResourceInfo resource_info = parse_resource_info(asset_data);
						if (resource_info.is_valid() == false)
						{
							qCritical() << "Invalid data in resource asset!";
							return;
						}

						if (Resource::is_resource_base_of_type(m_application, resource_info.type) == false)
						{
							qCritical() << "Resource asset does not contain valid type!";
							return;
						}

						// Validate the resource contents
						switch (asset_info.type)
						{
						case Core::AssetType::SCENE:
						{
							if (resource_info.type != Utilities::base64_string_to_uuid(::Vadon::Foundation::SceneSchema::c_type_uuid.string))
							{
								qCritical() << "Scene asset does not contain scene type!";
								return;
							}
						}
						break;
						case Core::AssetType::IMPORTED_FILE:
						{
							if (resource_info.type != Utilities::base64_string_to_uuid(::Vadon::Foundation::FileResourceSchema::c_type_uuid.string))
							{
								qCritical() << "Imported file does not contain imported file resource type!";
								return;
							}
						}
						break;
						}

						if (m_resource_asset_lookup.find(resource_info.id) != m_resource_asset_lookup.end())
						{
							qCritical() << "Resource ID already present in another asset!";
							return;
						}

						if (m_resource_asset_reverse_lookup.find(asset_info.id) != m_resource_asset_reverse_lookup.end())
						{
							qCritical() << "Asset ID already registered!";
							return;
						}

						m_resource_asset_lookup.insert(resource_info.id, asset_info.id);
						m_resource_asset_reverse_lookup.insert(asset_info.id, resource_info);
					}
						break;
					case Core::AssetType::FOLDER:
						// Process folder contents recursively
						self(self, current_child);
						break;
					}
				}
			};

			return do_process_assets_recursive(do_process_assets_recursive, current_root);
		};

		process_assets_recursive(root_item);
	}

	void ResourceSystem::shutdown()
	{
		// Clean up any leftover resources
		for (auto resource_it = m_resource_lookup.begin(); resource_it != m_resource_lookup.end(); ++resource_it)
		{
			Resource* current_resource = resource_it.value();

			// Set flag to skip internal management, everything will be removed
			current_resource->m_pending_remove = true;

			delete current_resource;
		}

		m_resource_lookup.clear();
		m_resource_asset_lookup.clear();
		m_resource_asset_reverse_lookup.clear();
		m_resource_init_data_lookup.clear();
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

		if (Resource::is_resource_base_of_type(m_application, info.type) == false)
		{
			qCritical("Content type is not a subclass of Resource!");
			return false;
		}

		return true;
	}

	bool ResourceSystem::internal_add_resource_asset(const ResourceInfo& info, int asset_id)
	{
		const int prev_asset_id = find_resource_asset_id(info.id);
		if (prev_asset_id != Core::AssetInfo::c_invalid_file_id)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::internal_add_resource_asset", "Resource asset already added");
			return false;
		}

		const ResourceInfo prev_info = resource_info_by_asset_id(asset_id);
		if (prev_info.is_valid() == true)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::internal_add_resource_asset", "Resource asset already added");
			return false;
		}

		m_resource_asset_lookup.insert(info.id, asset_id);
		m_resource_asset_reverse_lookup.insert(asset_id, info);

		return true;
	}
}