#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <Vadon/Utilities/Enum/EnumClass.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/MemberBind.hpp>

#include <format>

namespace
{
	struct ResourceLoader
	{
		Vadon::Utilities::Serializer::Instance serializer_instance;

		bool initialize(Vadon::Core::EngineCoreInterface& engine_core, const Vadon::Scene::ResourcePath& path)
		{
			if (path.is_valid() == false)
			{
				engine_core.log_error("Resource system: cannot load a resource without a valid path!\n");
				return false;
			}

			Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
			Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
			if (file_system.load_file(path, resource_file_buffer) == false)
			{
				engine_core.log_error("Resource system: failed to load resource file!\n");
				return false;
			}

			// FIXME: support binary file serialization!
			// Solution: have file system create the appropriate serializer!
			serializer_instance = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

			if (serializer_instance->initialize() == false)
			{
				engine_core.log_error("Resource system: failed to initialize serializer while loading resource!\n");
				return false;
			}

			return true;
		}

		void finalize(Vadon::Core::EngineCoreInterface& engine_core)
		{
			if (serializer_instance->finalize() == false)
			{
				engine_core.log_error("Resource system: failed to finalize serializer after loading resource!\n");
			}
		}
	};

	void invalid_resource_data_error()
	{
		// TODO: print absolute path!
		Vadon::Core::Logger::log_error("Resource system error: invalid resource data!\n");
	}

	void resource_info_failed_to_serialize()
	{
		// TODO: print resource UUID!
		Vadon::Core::Logger::log_error("Resource system: failed to serialize resource info!\n");
	}

	void resource_data_failed_to_serialize()
	{
		// TODO: print resource UUID!
		Vadon::Core::Logger::log_error("Resource system: failed to serialize resource data!\n");
	}

	void resource_custom_data_failed_to_serialize()
	{
		// TODO: print resource UUID!
		Vadon::Core::Logger::log_error("Resource system: failed to serialize resource custom data!\n");
	}

	template<typename T>
	constexpr size_t variant_type_list_index_v = Vadon::Utilities::type_list_index_v<T, Vadon::Utilities::Variant>;

	// FIXME: deduplicate between this and SceneSystem!
	// TODO: default value?
	template<typename T>
	bool serialize_trivial_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value, Vadon::Utilities::Serializer::Result& result, Vadon::Utilities::ErasedDataTypeID data_type)
	{
		if (data_type.id != variant_type_list_index_v<T>)
		{
			return false;
		}

		if (serializer.is_reading() == true)
		{
			// TODO: allow setting default value if not found?
			T value;
			result = serializer.serialize(property_name, value);
			if (result == Vadon::Utilities::Serializer::Result::SUCCESSFUL)
			{
				property_value = value;
			}
		}
		else
		{
			T& value = std::get<T>(property_value);
			result = serializer.serialize(property_name, value);
		}

		return true;
	}

	template <typename... Types>
	Vadon::Utilities::Serializer::Result serialize_trivial_property_fold(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value, Vadon::Utilities::ErasedDataTypeID data_type)
	{
		Vadon::Utilities::Serializer::Result result;
		const bool fold_result = (serialize_trivial_property<Types>(serializer, property_name, property_value, result, data_type) || ...);
		if (fold_result == false)
		{
			result = Vadon::Utilities::Serializer::Result::NOT_IMPLEMENTED;
		}

		return result;
	}

	Vadon::Utilities::Serializer::Result process_trivial_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value, Vadon::Utilities::ErasedDataTypeID data_type)
	{
		return serialize_trivial_property_fold<int, float, bool, std::string, Vadon::Utilities::Vector2, Vadon::Utilities::Vector2i, Vadon::Utilities::Vector3, Vadon::Utilities::UUID>(serializer, property_name, property_value, data_type);
	}
}

namespace Vadon::Private::Scene
{
	ResourceHandle ResourceSystem::create_resource(Vadon::Utilities::TypeID type_id)
	{
		ResourceBase* resource = internal_create_resource(type_id);
		if (resource == nullptr)
		{
			return ResourceHandle();
		}

		// Generate UUID, checking to make sure it doesn't collide
		// TODO: have a cutoff where we stop trying again?
		ResourceID new_resource_id;
		while (true)
		{
			new_resource_id = ResourceID::generate();
			if (find_resource(new_resource_id).is_valid() == false)
			{
				break;
			}
		}

		return internal_add_resource(ResourceInfo{ .id = new_resource_id, .type_id = type_id }, resource);
	}

	ResourceHandle ResourceSystem::find_resource(ResourceID resource_id) const
	{
		auto resource_handle_it = m_resource_lookup.find(resource_id);
		if (resource_handle_it == m_resource_lookup.end())
		{
			return ResourceHandle();
		}

		return resource_handle_it->second;
	}

	ResourceInfo ResourceSystem::get_resource_info(ResourceHandle resource_handle) const
	{
		const ResourceData& resource_data = m_resource_pool.get(resource_handle);
		return resource_data.info;
	}

	void ResourceSystem::set_resource_path(ResourceHandle resource_handle, const ResourcePath& path)
	{
		ResourceData& resource_data = m_resource_pool.get(resource_handle);
		resource_data.info.path = path;
	}

	std::vector<ResourceHandle> ResourceSystem::find_resources_of_type(Vadon::Utilities::TypeID type_id) const
	{
		std::vector<ResourceHandle> result;

		for (const auto& resource_obj_pair : m_resource_pool)
		{
			const ResourceData* current_resource_data = resource_obj_pair.second;
			if (Vadon::Utilities::TypeRegistry::is_base_of(type_id, current_resource_data->info.type_id) == true)
			{
				result.push_back(resource_obj_pair.first);
			}
		}

		return result;
	}

	ResourceHandle ResourceSystem::import_resource(const ResourcePath& path)
	{
		ResourceLoader resource_loader;

		if (resource_loader.initialize(m_engine_core, path) == false)
		{
			log_error("Resource system: failed to import resource file!\n");
			return ResourceHandle();
		}

		ResourceInfo resource_info;
		if (internal_load_resource_info(*resource_loader.serializer_instance, resource_info) == false)
		{
			log_error("Resource system: failed to import resource info from file!\n");
			return ResourceHandle();
		}

		resource_loader.finalize(m_engine_core);

		ResourceHandle imported_resource_handle = find_resource(resource_info.id);
		if (imported_resource_handle.is_valid() == true)
		{
			// Resource already imported
			// TODO: log/warning message?
			return imported_resource_handle;
		}

		// Store path and add resource to pool
		resource_info.path = path;
		imported_resource_handle = internal_add_resource(resource_info, nullptr);

		return imported_resource_handle;
	}

	ResourceHandle ResourceSystem::load_resource(const ResourcePath& path)
	{
		ResourceLoader resource_loader;

		if (resource_loader.initialize(m_engine_core, path) == false)
		{
			log_error("Resource system: failed to load resource file!\n");
			return ResourceHandle();
		}

		ResourceHandle loaded_resource_handle;
		internal_load_resource(*resource_loader.serializer_instance, loaded_resource_handle);

		resource_loader.finalize(m_engine_core);

		if (loaded_resource_handle.is_valid() == true)
		{
			// Load successful, save the path
			ResourceData& loaded_resource_data = m_resource_pool.get(loaded_resource_handle);
			if (loaded_resource_data.info.path.is_valid() == false)
			{
				loaded_resource_data.info.path = path;
			}
			else
			{
				// TODO: display resource ID and path!
				log_error("Resource system: loaded resource with existing ID from different path!\n");
			}
		}

		return loaded_resource_handle;
	}

	bool ResourceSystem::save_resource(ResourceHandle resource_handle)
	{
		ResourceData& resource = m_resource_pool.get(resource_handle);
		if(resource.info.path.is_valid() == false)
		{
			log_error("Resource system: cannot save resource without a valid path!\n");
			return false;
		}

		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::WRITE);

		if (serializer->initialize() == false)
		{
			log_error("Resource system: failed to initialize serializer while saving resource!\n");
			return false;
		}

		if (internal_save_resource(*serializer, resource_handle) == false)
		{
			log_error("Resource system: failed to serialize resource data!\n");
			return false;
		}

		if (serializer->finalize() == false)
		{
			log_error("Resource system: failed to finalize serializer after saving resource!\n");
			return false;
		}

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();
		if (file_system.save_file(resource.info.path, resource_file_buffer) == false)
		{
			log_error("Resource system: failed to save resource data to file!\n");
			return false;
		}

		return true;
	}

	bool ResourceSystem::load_resource(ResourceHandle resource_handle)
	{
		// TODO: allow force-reloading?
		ResourceData& resource_data = m_resource_pool.get(resource_handle);
		if (resource_data.is_loaded() == true)
		{
			return true;
		}

		if (resource_data.info.path.is_valid() == false)
		{
			log_error("Resource system: cannot load resource without a valid path!\n");
			return false;
		}

		ResourceLoader resource_loader;

		if (resource_loader.initialize(m_engine_core, resource_data.info.path) == false)
		{
			return false;
		}

		resource_data.resource = internal_load_resource_data(*resource_loader.serializer_instance, resource_data.info);
		resource_loader.finalize(m_engine_core);

		return resource_data.is_loaded();
	}

	bool ResourceSystem::serialize_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle& resource_handle)
	{
		if (serializer.is_reading() == true)
		{
			resource_handle.invalidate();
			return internal_load_resource(serializer, resource_handle);
		}
		else
		{
			return internal_save_resource(serializer, resource_handle);
		}
	}

	bool ResourceSystem::serialize_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, ResourceHandle& property_value)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;
		if (serializer.is_reading() == true)
		{
			ResourceID resource_id;
			if (serializer.serialize(property_name, resource_id) == SerializerResult::SUCCESSFUL)
			{
				if (resource_id.is_valid() == true)
				{
					// TODO: should we try loading the resource from file if we can't find it?
					const ResourceHandle resource_handle = find_resource(resource_id);
					if (resource_handle.is_valid() == false)
					{
						// FIXME: show resource ID!
						log_error("Resource system: cannot find referenced resource!\n");
						return false;
					}
					property_value = resource_handle;
				}
				else
				{
					property_value.invalidate();
				}
			}
			else
			{
				// TODO: should use serializer codes to indicate whether an error occurred, or the property was just not present
				return false;
			}
		}
		else
		{
			ResourceID resource_id = property_value.is_valid() ? get_resource_info(property_value).id : ResourceID();
			if (serializer.serialize(property_name, resource_id) != SerializerResult::SUCCESSFUL)
			{
				log_error(std::format("Resource system: unable to serialize resource property \"{}\"!\n", property_name));
				return false;
			}
		}

		return true;
	}

	void ResourceSystem::unload_resource(ResourceHandle resource_handle)
	{
		ResourceData& resource_data = m_resource_pool.get(resource_handle);
		if (resource_data.is_loaded() == true)
		{
			delete resource_data.resource;
			resource_data.resource = nullptr;
		}
	}

	void ResourceSystem::remove_resource(ResourceHandle resource_handle)
	{
		unload_resource(resource_handle);
		m_resource_pool.remove(resource_handle);
	}

	bool ResourceSystem::is_resource_loaded(ResourceHandle resource_handle) const
	{
		return m_resource_pool.get(resource_handle).is_loaded();
	}

	const ResourceBase* ResourceSystem::get_resource_base(ResourceHandle resource_handle) const
	{
		return m_resource_pool.get(resource_handle).resource;
	}

	ResourceSystem::ResourceSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Scene::ResourceSystem(core)
	{}

	bool ResourceSystem::initialize()
	{
		log_message("Initializing Resource System\n");

		Vadon::Scene::ResourceRegistry::register_resource_type<ResourceBase>();
		Vadon::Utilities::TypeRegistry::add_property<ResourceBase>("name", Vadon::Utilities::MemberVariableBind<&ResourceBase::name>().bind_member_getter().bind_member_setter());

		log_message("Resource System initialized successfully!\n");
		return true;
	}

	void ResourceSystem::shutdown()
	{
		log_message("Shutting down Resource System\n");
		for (auto resource_pair : m_resource_pool)
		{
			ResourceData* current_resource_data = resource_pair.second;
			if (current_resource_data->is_loaded() == true)
			{
				delete current_resource_data->resource;
				current_resource_data->resource = nullptr;
			}
		}
		log_message("Resource System shut down!\n");
	}

	bool ResourceSystem::internal_load_resource_info(Vadon::Utilities::Serializer& serializer, ResourceInfo& info) const
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;

		if (serializer.is_reading() == false)
		{
			log_message("Resource system: invalid serializer!\n");
			return false;
		}

		ResourceID resource_id;
		if (serializer.serialize("id", info.id) != SerializerResult::SUCCESSFUL)
		{
			resource_info_failed_to_serialize();
			return false;
		}

		std::string resource_type;
		if (serializer.serialize("type", resource_type) != SerializerResult::SUCCESSFUL)
		{
			resource_info_failed_to_serialize();
			return false;
		}

		info.type_id = Vadon::Utilities::TypeRegistry::get_type_id(resource_type);
		if (info.type_id == Vadon::Utilities::TypeID::INVALID)
		{
			log_error(std::format("Resource system error: resource file uses unknown type \"{}\"!\n", resource_type));
			return false;
		}

		return true;
	}

	bool ResourceSystem::internal_save_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle resource_handle)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;
		using ErasedDataType = Vadon::Utilities::ErasedDataType;

		ResourceData& resource_data = m_resource_pool.get(resource_handle);

		if (resource_data.is_loaded() == false)
		{
			log_error("Resource system: cannot save resource that has not been loaded!\n");
			return false;
		}

		if (serializer.serialize("id", resource_data.info.id) != SerializerResult::SUCCESSFUL)
		{
			resource_info_failed_to_serialize();
			return false;
		}

		// Get type
		std::string resource_type = Vadon::Utilities::TypeRegistry::get_type_info(resource_data.info.type_id).name;
		if (serializer.serialize("type", resource_type) != SerializerResult::SUCCESSFUL)
		{
			resource_info_failed_to_serialize();
			return false;
		}

		// Serialize properties
		if (serializer.open_object("properties") != SerializerResult::SUCCESSFUL)
		{
			resource_data_failed_to_serialize();
			return false;
		}

		Vadon::Utilities::PropertyList resource_properties = Vadon::Utilities::TypeRegistry::get_properties(resource_data.resource, resource_data.info.type_id);

		for (Vadon::Utilities::Property& current_property : resource_properties)
		{
			switch (current_property.data_type.type)
			{
			case ErasedDataType::TRIVIAL:
			{
				const SerializerResult result = process_trivial_property(serializer, current_property.name, current_property.value, current_property.data_type);
				if (result != SerializerResult::SUCCESSFUL)
				{
					invalid_resource_data_error();
				}
			}
			break;
			case ErasedDataType::RESOURCE_HANDLE:
			{
				ResourceHandle temp_resource_handle = std::get<ResourceHandle>(current_property.value);
				if (serialize_resource_property(serializer, current_property.name, temp_resource_handle) == false)
				{
					invalid_resource_data_error();
				}
			}
			break;
			}
		}

		if (serializer.close_object() != SerializerResult::SUCCESSFUL)
		{
			resource_data_failed_to_serialize();
			return false;
		}

		Vadon::Scene::ResourceRegistry::SerializerFunction resource_serializer = Vadon::Scene::ResourceRegistry::get_resource_serializer(resource_data.info.type_id);
		if (resource_serializer != nullptr)
		{
			if (serializer.open_object("data") != SerializerResult::SUCCESSFUL)
			{
				resource_custom_data_failed_to_serialize();
				return false;
			}
			if (resource_serializer(*this, serializer, *resource_data.resource) == false)
			{
				resource_custom_data_failed_to_serialize();
				return false;
			}
			if (serializer.close_object() != SerializerResult::SUCCESSFUL)
			{
				resource_custom_data_failed_to_serialize();
				return false;
			}
		}

		return true;
	}

	bool ResourceSystem::internal_load_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle& resource_handle)
	{
		ResourceInfo info;
		if (internal_load_resource_info(serializer, info) == false)
		{
			return false;
		}

		resource_handle = find_resource(info.id);
		if (resource_handle.is_valid() == true)
		{
			// Resource already registered
			ResourceData& resource_data = m_resource_pool.get(resource_handle);
			if (resource_data.is_loaded() == true)
			{
				// Resource already loaded
				// TODO: log/warning message?
				return true;
			}
			else
			{
				resource_data.resource = internal_load_resource_data(serializer, info);
				return resource_data.is_loaded();
			}
		}

		// Resource is loaded for the first time
		// Try loading the resource data first
		ResourceBase* loaded_resource = internal_load_resource_data(serializer, info);
		if (loaded_resource == nullptr)
		{
			return false;
		}

		resource_handle = internal_add_resource(info, loaded_resource);
		return true;
	}

	ResourceBase* ResourceSystem::internal_load_resource_data(Vadon::Utilities::Serializer& serializer, const ResourceInfo& info)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;
		using ErasedDataType = Vadon::Utilities::ErasedDataType;

		// Create resource object
		std::unique_ptr<ResourceBase> resource(internal_create_resource(info.type_id));
		if (resource == nullptr)
		{
			return nullptr;
		}

		// Deserialize resource data
		if (serializer.open_object("properties") != SerializerResult::SUCCESSFUL)
		{
			resource_data_failed_to_serialize();
			return nullptr;
		}

		const Vadon::Utilities::PropertyInfoList resource_properties = Vadon::Utilities::TypeRegistry::get_type_properties(info.type_id);

		Vadon::Utilities::Variant current_property_value;
		for (const Vadon::Utilities::PropertyInfo& current_property_info : resource_properties)
		{
			switch (current_property_info.data_type.type)
			{
			case ErasedDataType::TRIVIAL:
			{
				const SerializerResult result = process_trivial_property(serializer, current_property_info.name, current_property_value, current_property_info.data_type);
				if (result == SerializerResult::SUCCESSFUL)
				{
					Vadon::Utilities::TypeRegistry::set_property(resource.get(), info.type_id, current_property_info.name, current_property_value);
				}
				else
				{
					invalid_resource_data_error();
				}
			}
			break;
			case ErasedDataType::RESOURCE_HANDLE:
			{
				ResourceHandle temp_resource_handle;
				if (serialize_resource_property(serializer, current_property_info.name, temp_resource_handle) == true)
				{
					current_property_value = temp_resource_handle;
					Vadon::Utilities::TypeRegistry::set_property(resource.get(), info.type_id, current_property_info.name, current_property_value);
				}
				else
				{
					invalid_resource_data_error();
				}
			}
			break;
			}
		}

		if (serializer.close_object() != SerializerResult::SUCCESSFUL)
		{
			resource_data_failed_to_serialize();
			return nullptr;
		}

		Vadon::Scene::ResourceRegistry::SerializerFunction resource_serializer = Vadon::Scene::ResourceRegistry::get_resource_serializer(info.type_id);
		if (resource_serializer != nullptr)
		{
			if (serializer.open_object("data") != SerializerResult::SUCCESSFUL)
			{
				resource_custom_data_failed_to_serialize();
				return nullptr;
			}
			if (resource_serializer(*this, serializer, *resource) == false)
			{
				resource_custom_data_failed_to_serialize();
				return nullptr;
			}
			if (serializer.close_object() != SerializerResult::SUCCESSFUL)
			{
				resource_custom_data_failed_to_serialize();
				return nullptr;
			}
		}

		// Load succeeded, unique_ptr can release
		return resource.release();
	}

	ResourceBase* ResourceSystem::internal_create_resource(Vadon::Utilities::TypeID type_id) const
	{
		ResourceBase* new_resource = Vadon::Scene::ResourceRegistry::create_resource(type_id);
		if (new_resource == nullptr)
		{
			// FIXME: show type name?
			log_error(std::format("Resource system error: failed to create resource with type ID {}!\n", Vadon::Utilities::to_integral(type_id)));
		}

		return new_resource;
	}

	ResourceHandle ResourceSystem::internal_add_resource(const ResourceInfo& info, ResourceBase* resource)
	{
		// Create object, add to lookup
		ResourceHandle new_resource_handle = m_resource_pool.add();
		m_resource_lookup[info.id] = new_resource_handle;

		// Set metadata
		ResourceData& new_resource_data = m_resource_pool.get(new_resource_handle);
		new_resource_data.info = info;
		new_resource_data.resource = resource;

		return new_resource_handle;
	}
}