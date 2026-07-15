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

		Resource* new_resource = internal_create_new_resource(new_resource_info);
		if (new_resource != nullptr)
		{
			internal_add_new_resource(new_resource);
		}

		return new_resource;
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

		auto cached_info_it = m_resource_asset_reverse_lookup.find(resource_asset_id);
		if (cached_info_it == m_resource_asset_reverse_lookup.end())
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::get_resource", "Cannot find cached resource info!");
			return nullptr;
		}

		const ResourceInfo& resource_info = cached_info_it.value();
		Resource* new_resource = internal_create_new_resource(resource_info);

		if (internal_load_resource(new_resource, resource_asset_id) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::get_resource", "Failed to load resource data!");
			delete new_resource;
			return nullptr;
		}

		internal_add_new_resource(new_resource);
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

	void ResourceSystem::save_all_resources()
	{
		for (auto resource_it = m_resource_lookup.begin(); resource_it != m_resource_lookup.end(); ++resource_it)
		{
			const Resource* current_resource = resource_it.value();
			if (current_resource->is_embedded() == true)
			{
				continue;
			}

			if (current_resource->is_modified() == true)
			{
				if (save_resource(current_resource) == false)
				{
					qCritical() << "Failed to save resource!";
				}
			}
		}
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

		const Core::AssetType asset_type = get_asset_type_for_resource_type(resource->get_info().type);

		Core::AssetInfo resource_asset_info;
		resource_asset_info.path = Core::AssetInfo::get_file_path(path, asset_type);
		resource_asset_info.type = asset_type;

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

		// Clear the modified flag
		// FIXME: this is a bit hacky, we re-query the resource to get a non-const pointer
		find_resource(resource->get_info().id)->clear_modified();
		return true;
	}

	bool ResourceSystem::reload_resource(Resource* resource)
	{
		// First find the asset ID
		const int asset_id = find_resource_asset_id(resource->get_info().id);
		if (asset_id == Core::AssetInfo::c_invalid_file_id)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::reload_resource", "Cannot find resource asset");
			return false;
		}

		// Initialize to clear any changes
		if (resource->initialize() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::reload_resource", "Failed to initialize resource!");
			return false;
		}

		// Finally reload from asset
		if (internal_load_resource(resource, asset_id) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::reload_resource", "Failed to load resource!");
			return false;
		}

		resource->clear_modified();
		return true;
	}

	Resource* ResourceSystem::import_file_resource(int asset_id)
	{
		if (m_resource_asset_reverse_lookup.find(asset_id) != m_resource_asset_reverse_lookup.end())
		{
			qCritical() << "Resource asset already added!";
			return nullptr;
		}

		Resource* import_resource = create_resource(Resource::get_imported_file_resource_type());
		if (internal_add_resource_asset(import_resource->get_info(), asset_id) == false)
		{
			qCritical() << "Failed to register import resource asset!";
			delete import_resource;
			return nullptr;
		}

		if (save_resource(import_resource) == false)
		{
			qCritical() << "Failed to save import resource data!";
			delete import_resource;
			return nullptr;
		}

		return import_resource;
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
							if (resource_info.type != Scene::get_scene_type_uuid())
							{
								qCritical() << "Scene asset does not contain scene type!";
								return;
							}
						}
						break;
						case Core::AssetType::IMPORTED_FILE:
						{
							if (resource_info.type != Resource::get_imported_file_resource_type())
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

		return new_resource;
	}

	void ResourceSystem::internal_add_new_resource(Resource* resource)
	{
		Q_ASSERT_X(resource != nullptr, "VadonEditor::Model::ResourceSystem::internal_add_new_resource", "Resource must not be null!");
		Q_ASSERT_X(resource->get_info().is_valid() == true, "VadonEditor::Model::ResourceSystem::internal_add_new_resource", "Resource must be valid!");
		Q_ASSERT_X(m_resource_lookup.find(resource->get_info().id) == m_resource_lookup.end(), "VadonEditor::Model::ResourceSystem::internal_add_new_resource", "Resource already added!");

		m_resource_lookup.insert(resource->get_info().id, resource);
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

	bool ResourceSystem::internal_load_resource(Resource* resource, int asset_id)
	{
		QByteArray resource_file_data;
		if (m_application.get_asset_manager().load_asset_data(asset_id, resource_file_data) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::internal_load_resource", "Unable to load resource file data!");
			return false;
		}

		// TODO: check error 
		QJsonDocument json_document = QJsonDocument::fromJson(resource_file_data);
		if (json_document.isNull() == true)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::internal_load_resource", "Resource file is not valid JSON!");
			return false;
		}

		// Parse resource info
		// FIXME: have a LUT of imported resources so we don't have to do this more than once!
		const QJsonObject& root_object = json_document.object();
		if (resource->internal_load(root_object) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::internal_load_resource", "Failed to load resource data!");
			return false;
		}

		// NOTE: we register all embedded resources after the fact, this ensures we don't expose the API
		internal_add_embedded_resources(resource);
		return true;
	}

	void ResourceSystem::internal_add_embedded_resources(Resource* resource)
	{
		if (resource->m_embedded_resources.isEmpty() == true)
		{
			return;
		}

		for (auto embedded_it = resource->m_embedded_resources.begin(); embedded_it != resource->m_embedded_resources.end(); ++embedded_it)
		{
			internal_add_new_resource(embedded_it.value());

			// Recursively add any embedded resources
			internal_add_embedded_resources(embedded_it.value());
		}
	}
}