#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Debugging/Assert.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
#include <Vadon/Foundation/Model/Resource/File.hpp>

#include <filesystem>

namespace
{
	std::string get_filesystem_style_asset_extension(::Vadon::Foundation::ResourceFileInfo::Type file_type)
	{
		return std::format(".{}", ::Vadon::Foundation::ResourceFileInfo::get_file_extension(file_type));
	}

	::Vadon::Foundation::ResourceFileInfo::Type get_file_asset_type(const std::filesystem::path& path)
	{
		const std::string extension = path.extension().generic_string();
		if (extension == get_filesystem_style_asset_extension(::Vadon::Foundation::ResourceFileInfo::Type::SCENE))
		{
			return ::Vadon::Foundation::ResourceFileInfo::Type::SCENE;
		}
		else if (extension == get_filesystem_style_asset_extension(::Vadon::Foundation::ResourceFileInfo::Type::RESOURCE))
		{
			return ::Vadon::Foundation::ResourceFileInfo::Type::RESOURCE;
		}
		else if (extension == get_filesystem_style_asset_extension(::Vadon::Foundation::ResourceFileInfo::Type::IMPORTED_FILE))
		{
			return ::Vadon::Foundation::ResourceFileInfo::Type::IMPORTED_FILE;
		}

		return ::Vadon::Foundation::ResourceFileInfo::Type::NONE;
	}
}

namespace VadonEditor::Model
{
	ResourceSystem::~ResourceSystem()
	{
		VADON_ASSERT(m_resource_lookup.empty() == true, "Resources were not cleared");
	}

	const Resource* ResourceSystem::find_resource(const Vadon::Model::ResourceID& resource_id) const
	{
		auto resource_it = m_resource_lookup.find(resource_id);
		if (resource_it != m_resource_lookup.end())
		{
			return resource_it->second;
		}

		return nullptr;
	}

	Resource* ResourceSystem::get_resource(const Vadon::Model::ResourceID& resource_id)
	{
		Resource* resource_obj = find_resource(resource_id);
		if (resource_obj != nullptr)
		{
			return resource_obj;
		}
		
		const ResourceDatabaseEntry* resource_db_entry = m_database.find_resource_entry(resource_id);
		if (resource_db_entry == nullptr)
		{
			return nullptr;
		}

		// Create new resource object
		Resource* new_resource_obj = new Resource(m_editor, resource_id);
		if (new_resource_obj->initialize() == false)
		{
			VADON_ERROR("Failed to load resource data!");
			delete new_resource_obj;
			return nullptr;
		}

		internal_add_resource(new_resource_obj);
		return new_resource_obj;
	}

	void ResourceSystem::register_event_callback(EventCallback callback)
	{
		m_event_callbacks.push_back(callback);
	}

	void ResourceSystem::broadcast_resource_event(const ResourceEvent& event)
	{
		for (const EventCallback& current_callback : m_event_callbacks)
		{
			current_callback(event);
		}
	}

	ResourceSystem::ResourceSystem(Core::Editor& editor)
		: m_editor(editor)
		, m_database(editor)
	{

	}

	bool ResourceSystem::initialize()
	{
		// Register resource DB with engine
		Vadon::Model::ResourceSystem& engine_resource_system = m_editor.get_engine_core().get_system<Vadon::Model::ResourceSystem>();
		engine_resource_system.register_database(m_database);

		return true;
	}

	void ResourceSystem::shutdown()
	{
		for (const auto resource_pair : m_resource_lookup)
		{
			Resource* current_resource = resource_pair.second;
			current_resource->shutdown();

			delete current_resource;
		}

		m_resource_lookup.clear();
	}

	bool ResourceSystem::project_loaded()
	{
		// Initialize the database
		if (m_database.initialize() == false)
		{
			return false;
		}

		// Import all resources in the project
		// FIXME: make use of a cache so we don't have to load every resource to get its ID
		// Will need to check whether something changed between the cache and the actual files
		Core::ProjectManager& project_manager = m_editor.get_project_manager();
		const std::filesystem::path root_fs_path(project_manager.get_active_project().root_path);

		bool all_valid = true;

		for (const auto& directory_entry : std::filesystem::recursive_directory_iterator(root_fs_path))
		{
			if (directory_entry.is_regular_file() == false)
			{
				continue;
			}

			const ::Vadon::Foundation::ResourceFileInfo::Type current_asset_type = get_file_asset_type(directory_entry.path());
			if (current_asset_type != ::Vadon::Foundation::ResourceFileInfo::Type::NONE)
			{
				const std::string relative_path = std::filesystem::relative(directory_entry.path(), root_fs_path).generic_string();
				all_valid &= m_database.import_resource(relative_path).is_valid();
			}
		}

		return all_valid;
	}

	void ResourceSystem::process_message(const char* data, size_t size)
	{
		::Vadon::Foundation::EditorMessageReader message_reader(data, size);
		switch (message_reader.get_current_category())
		{
		case ::Vadon::Foundation::EditorMessageCategory::MODEL:
		{
			const char* message_data = message_reader.get_current_message_data();
			const ::Vadon::Foundation::EditorModelMessageHeader* model_message_header = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageHeader*>(message_data);
			switch (model_message_header->message_type)
			{
			case ::Vadon::Foundation::EditorModelMessageType::RESOURCE_CREATED:
			{
				const ::Vadon::Foundation::EditorModelMessageResourceCreated* resource_created_message = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageResourceCreated*>(message_data);
				internal_create_resource(resource_created_message->type_id, resource_created_message->resource_id);
			}
			break;
			case ::Vadon::Foundation::EditorModelMessageType::RESOURCE_ASSET_CREATED:
			{
				const ::Vadon::Foundation::EditorModelMessageResourceAssetCreated* resource_asset_created = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageResourceAssetCreated*>(message_data);

				Resource* resource = find_resource(resource_asset_created->resource_id);
				VADON_ASSERT(resource != nullptr, "Resource was not loaded!");

				const char* data_start = message_data + sizeof(::Vadon::Foundation::EditorModelMessageResourceAssetCreated);

				std::string_view asset_path_string(data_start, resource_asset_created->asset_path_length);

				const Vadon::Model::ResourceID imported_id = m_database.import_resource(asset_path_string);
				VADON_ASSERT(imported_id == resource_asset_created->resource_id, "Resource in asset does not match Resource object");
			}
			break;
			case ::Vadon::Foundation::EditorModelMessageType::RESOURCE_LOADED:
			{
				const ::Vadon::Foundation::EditorModelMessageResourceLoaded* resource_loaded_message = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageResourceLoaded*>(message_data);

				Resource* resource = find_resource(resource_loaded_message->resource_id);
				if (resource == nullptr)
				{
					// Resource not yet loaded, so we create it
					resource = get_resource(resource_loaded_message->resource_id);
					VADON_ASSERT(resource != nullptr, "Failed to create resource!");

					Vadon::Core::Logger::log_message(std::format("Loaded resource {}\n", Vadon::Utilities::uuid_to_string(resource->get_id()).string));
				}
				else if (resource_loaded_message->reload == true)
				{
					reload_resource(resource);
				}
			}
			break;
			case ::Vadon::Foundation::EditorModelMessageType::RESOURCE_PROPERTY_EDITED:
			{
				const ::Vadon::Foundation::EditorModelMessageResourcePropertyEdited* resource_property_edited = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageResourcePropertyEdited*>(message_data);

				Resource* resource = find_resource(resource_property_edited->resource_id);
				VADON_ASSERT(resource != nullptr, "Resource was not loaded!");

				resource->load_property_data(*resource_property_edited, message_data);
			}
			break;
			case ::Vadon::Foundation::EditorModelMessageType::RESOURCE_REMOVED:
				// TODO: remove resource object
				// TODO2: if embedded, we should not be getting this message!
				break;
			case ::Vadon::Foundation::EditorModelMessageType::RESOURCE_ADD_EMBEDDED:
			{
				const ::Vadon::Foundation::EditorModelMessageResourceAddEmbedded* add_embedded = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageResourceAddEmbedded*>(message_data);

				Resource* resource = find_resource(add_embedded->resource_id);
				VADON_ASSERT(resource != nullptr, "Resource was not loaded!");
				
				Resource* embedded_resource = find_resource(add_embedded->embedded_id);
				VADON_ASSERT(embedded_resource != nullptr, "Cannot find embedded resource!");

				if (resource->add_embedded_resource(embedded_resource) == false)
				{
					VADON_ERROR("Failed to add embedded resource!");
				}
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::RESOURCE_REMOVE_EMBEDDED:
			{
				const ::Vadon::Foundation::EditorModelMessageResourceRemoveEmbedded* remove_embedded = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageResourceRemoveEmbedded*>(message_data);

				Resource* resource = find_resource(remove_embedded->resource_id);
				VADON_ASSERT(resource != nullptr, "Resource was not loaded!");

				Resource* embedded_resource = find_resource(remove_embedded->embedded_id);
				VADON_ASSERT(embedded_resource != nullptr, "Embedded not found!");

				resource->remove_embedded_resource(embedded_resource);

				// This time we force removal
				internal_remove_resource(embedded_resource, true);
			}
				break;
			}
		}
		break;
		}
	}

	void ResourceSystem::internal_create_resource(const Vadon::Utilities::TypeUUID& type_id, const Vadon::Model::ResourceID& resource_id)
	{
		VADON_ASSERT(find_resource(resource_id) == nullptr, "Resource with ID already exists!");

		Vadon::Model::ResourceSystem& engine_resource_system = m_editor.get_engine_core().get_system<Vadon::Model::ResourceSystem>();
		Vadon::Model::ResourceHandle resource_handle = engine_resource_system.create_resource_with_id(Vadon::Utilities::TypeRegistry::get_type_id(type_id), resource_id);
		VADON_ASSERT(resource_handle.is_valid() == true, "Failed to create resource!");

		// Create new resource object
		Resource* new_resource_obj = new Resource(m_editor, resource_id);
		new_resource_obj->m_handle = resource_handle;
		if (new_resource_obj->initialize() == false)
		{
			VADON_ERROR("Failed to load resource data!");
			delete new_resource_obj;
			return;
		}

		internal_add_resource(new_resource_obj);
	}

	void ResourceSystem::internal_add_resource(Resource* resource)
	{
		m_resource_lookup.insert(std::make_pair(resource->get_id(), resource));

		// Recursively add all embedded resources
		Vadon::Model::ResourceSystem& engine_resource_system = m_editor.get_engine_core().get_system<Vadon::Model::ResourceSystem>();
		const std::vector<Vadon::Model::ResourceHandle> embedded_resources = engine_resource_system.get_embedded_resources(resource->m_handle);

		for (Vadon::Model::ResourceHandle current_embedded_handle : embedded_resources)
		{
			const Vadon::Model::ResourceInfo embedded_info = engine_resource_system.get_resource_info(current_embedded_handle);

			Resource* embedded_resource_obj = new Resource(m_editor, embedded_info.id);
			embedded_resource_obj->m_info = embedded_info;
			embedded_resource_obj->m_handle = current_embedded_handle;
			embedded_resource_obj->m_owner = resource;

			internal_add_resource(embedded_resource_obj);
		}
	}

	void ResourceSystem::internal_remove_resource(Resource* resource, bool force_remove)
	{
		// Recursively remove all embedded resources
		for (Resource* current_embedded_resource : resource->m_embedded_resources)
		{
			VADON_ASSERT(find_resource(current_embedded_resource->get_id()) != nullptr, "Cannot find embedded resource!");
			internal_remove_resource(current_embedded_resource);
		}

		// Remove from lookup
		auto resource_it = m_resource_lookup.find(resource->get_id());
		VADON_ASSERT(resource_it != m_resource_lookup.end(), "Cannot find resource!");
		m_resource_lookup.erase(resource_it);

		// Remove from engine as well
		// NOTE: Normally we rely on the root resource kicking removal for all embedded resources,
		// unless we are explicitly removing a previously embedded resource
		if (resource->is_embedded() == false)
		{
			m_editor.get_engine_core().get_system<Vadon::Model::ResourceSystem>().remove_resource(resource->m_handle);
		}
		else if (force_remove == true)
		{
			m_editor.get_engine_core().get_system<Vadon::Model::ResourceSystem>().remove_embedded_resource(resource->m_owner->m_handle, resource->m_handle);
		}

		resource->shutdown();
		delete resource;
	}

	void ResourceSystem::reload_resource(Resource* resource)
	{
		VADON_ASSERT(resource->is_embedded() == false, "Cannot reload embedded resource!");

		const Vadon::Model::ResourceID resource_id = resource->get_id();

		// Notify listeners
		{
			ResourceEvent reload_event;
			reload_event.resource = resource_id;
			reload_event.type = ResourceEventType::RELOADED;

			broadcast_resource_event(reload_event);
		}

		// Remove resource to clean up stale data
		internal_remove_resource(resource);

		// Reload the same resource
		Resource* reloaded_resource = get_resource(resource_id);
		VADON_ASSERT(reloaded_resource != nullptr, "Failed to create resource!");

		Vadon::Core::Logger::log_message(std::format("Reloaded resource {}\n", Vadon::Utilities::uuid_to_string(reloaded_resource->get_id()).string));
	}
}