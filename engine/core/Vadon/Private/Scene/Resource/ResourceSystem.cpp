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
	void invalid_resource_data_error()
	{
		// TODO: print absolute path!
		Vadon::Core::Logger::log_error("Resource system error: invalid resource data!\n");
	}

	void resource_failed_to_serialize()
	{
		// TODO: print resource UUID!
		Vadon::Core::Logger::log_error("Resource system: failed to serialize resource data!\n");
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
	const ResourceBase* ResourceSystem::get_resource_base(ResourceHandle resource_handle) const
	{
		return m_resource_pool.get(resource_handle).resource;
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

	bool ResourceSystem::import_resource_library(Vadon::Core::RootDirectoryHandle root_directory, std::string_view extensions)
	{
		bool all_valid = true;

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();

		std::string extensions_string = extensions.empty() ? m_resource_file_extensions : std::string(extensions);
		
		const std::vector<ResourcePath> resource_files = file_system.get_files_of_type(ResourcePath{ .root_directory = root_directory }, extensions_string, true);
		for (const ResourcePath& current_file_path : resource_files)
		{
			all_valid &= import_resource_file(current_file_path).is_valid();
		}

		return all_valid;
	}

	ResourceHandle ResourceSystem::import_resource_file(const ResourcePath& path)
	{
		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();
		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;

		if (file_system.load_file(path, resource_file_buffer) == false)
		{
			log_error("Resource system: failed to load resource file!\n");
			return ResourceHandle();
		}

		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

		if (serializer->initialize() == false)
		{
			log_error("Resource system: failed to initialize serializer while loading resource!\n");
			return ResourceHandle();
		}

		ResourceHandle imported_resource_handle = import_resource(*serializer);
		if (imported_resource_handle.is_valid() == false)
		{
			log_error("Resource system: failed to import loaded resource!\n");
			return imported_resource_handle;
		}

		ResourceData& imported_resource_data = m_resource_pool.get(imported_resource_handle);
		imported_resource_data.info.path = path;

		if (serializer->finalize() == false)
		{
			log_error("Resource system: failed to finalize serializer after loading resource!\n");
		}

		return imported_resource_handle;
	}

	ResourceHandle ResourceSystem::import_resource(Vadon::Utilities::Serializer& serializer)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;

		// TODO: utility function for TypeIDs to be serialized as string and deserialized to the ID number
		ResourceID resource_id;
		if (serializer.serialize("id", resource_id) != SerializerResult::SUCCESSFUL)
		{
			resource_failed_to_serialize();
			return ResourceHandle();
		}

		// Check whether we already loaded this resource
		{
			ResourceHandle existing_resource = find_resource(resource_id);
			if (existing_resource.is_valid() == true)
			{
				// TODO: notification?
				return existing_resource;
			}
		}

		// Get type
		std::string resource_type;
		if (serializer.serialize("type", resource_type) != SerializerResult::SUCCESSFUL)
		{
			resource_failed_to_serialize();
			return ResourceHandle();
		}

		const Vadon::Utilities::TypeID resource_type_id = Vadon::Utilities::TypeRegistry::get_type_id(resource_type);
		if (resource_type_id == Vadon::Utilities::TypeID::INVALID)
		{
			log_error(std::format("Resource system error: resource data uses unknown type \"{}\"!\n", resource_type));
			return ResourceHandle();
		}

		return internal_register_resource(resource_id, resource_type_id);
	}

	bool ResourceSystem::save_resource(ResourceHandle resource_handle)
	{
		ResourceData& resource_data = m_resource_pool.get(resource_handle);
		if (resource_data.resource == nullptr)
		{
			// Resource must be loaded first!
			log_error("Resource system error: cannot save resource that isn't loaded!\n");
			return false;
		}

		if(resource_data.info.path.is_valid() == true)
		{
			// Cannot save a resource with no path!
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
		if (file_system.save_file(resource_data.info.path, resource_file_buffer) == false)
		{
			log_error("Resource system: failed to save resource data to file!\n");
			return false;
		}

		return true;
	}

	bool ResourceSystem::load_resource(ResourceHandle resource_handle)
	{
		ResourceData& resource_data = m_resource_pool.get(resource_handle);
		if(resource_data.is_loaded() == true)
		{
			// Resource already loaded!
			// TODO: notification?
			return true;
		}

		if (resource_data.info.path.is_valid() == false)
		{
			log_error("Resource system: cannot load a resource without a valid path!\n");
			return false;
		}

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();
		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
		if (file_system.load_file(resource_data.info.path, resource_file_buffer) == false)
		{
			log_error("Resource system: failed to load resource file!\n");
			return false;
		}

		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

		if (serializer->initialize() == false)
		{
			log_error("Resource system: failed to initialize serializer while loading resource!\n");
			return false;
		}

		if (internal_load_resource(*serializer, resource_handle) == false)
		{
			log_error("Resource system: failed to deserialize resource data!\n");
			return false;
		}

		if (serializer->finalize() == false)
		{
			log_error("Resource system: failed to finalize serializer after loading resource!\n");
			return false;
		}

		return true;
	}

	bool ResourceSystem::serialize_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle resource_handle)
	{
		if (serializer.is_reading() == true)
		{
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
				const ResourceHandle resource_handle = find_resource(resource_id);
				if (resource_handle.is_valid() == false)
				{
					// TODO: warning, or error?
					return false;
				}
				property_value = resource_handle;
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

	ResourceHandle ResourceSystem::internal_create_resource(ResourceBase* resource, Vadon::Utilities::TypeID type_id)
	{
		// Generate UUID, checking to make sure it doesn't collide
		ResourceID new_resource_id;
		while (true)
		{
			new_resource_id = ResourceID::generate();
			if(find_resource(new_resource_id).is_valid() == false)
			{
				break;
			}
		}

		ResourceHandle new_resource_handle = internal_register_resource(new_resource_id, type_id);

		ResourceData& new_resource_data = m_resource_pool.get(new_resource_handle);
		new_resource_data.resource = resource;

		return new_resource_handle;
	}

	ResourceSystem::ResourceSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Scene::ResourceSystem(core)
		, m_resource_file_extensions(".vdrc")
	{}

	bool ResourceSystem::initialize()
	{
		log_message("Initializing Resource System\n");
		Vadon::Scene::ResourceRegistry::register_resource_type<ResourceBase>();
		Vadon::Utilities::TypeRegistry::add_property<ResourceBase>("name", Vadon::Utilities::MemberVariableBind<&ResourceBase::name>().bind_member_getter().bind_member_setter());

		// TODO: add step where we import all resources in the current context!
		// Might need to be the responsibility of a system one layer above, e.g using a project file

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

	bool ResourceSystem::internal_load_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle resource_handle)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;
		using ErasedDataType = Vadon::Utilities::ErasedDataType;

		ResourceData& resource_data = m_resource_pool.get(resource_handle);
		if (resource_data.resource != nullptr)
		{
			// Resource already loaded
			// TODO: notification?
			return true;
		}

		// Create the resource
		if (internal_create_resource(resource_data) == false)
		{
			log_error("Resource system: failed to create loaded resource!\n");
			return false;
		}
		
		// Deserialize properties
		if (serializer.open_object("properties") != SerializerResult::SUCCESSFUL)
		{
			resource_failed_to_serialize();
			return false;
		}

		const Vadon::Utilities::PropertyInfoList resource_properties = Vadon::Utilities::TypeRegistry::get_type_properties(resource_data.info.type_id);

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
					Vadon::Utilities::TypeRegistry::set_property(resource_data.resource, resource_data.info.type_id, current_property_info.name, current_property_value);
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
					Vadon::Utilities::TypeRegistry::set_property(resource_data.resource, resource_data.info.type_id, current_property_info.name, current_property_value);
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
			resource_failed_to_serialize();
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
			resource_failed_to_serialize();
			return false;
		}

		// Get type
		std::string resource_type = Vadon::Utilities::TypeRegistry::get_type_info(resource_data.info.type_id).name;
		if (serializer.serialize("type", resource_type) != SerializerResult::SUCCESSFUL)
		{
			resource_failed_to_serialize();
			return false;
		}

		// Serialize properties
		if (serializer.open_object("properties") != SerializerResult::SUCCESSFUL)
		{
			resource_failed_to_serialize();
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
			resource_failed_to_serialize();
			return false;
		}

		return true;
	}

	ResourceHandle ResourceSystem::internal_register_resource(ResourceID resource_id, Vadon::Utilities::TypeID type_id)
	{
		// Create object, add to lookup
		ResourceHandle new_resource_handle = m_resource_pool.add();
		m_resource_lookup[resource_id] = new_resource_handle;

		// Set metadata
		ResourceData& new_resource_data = m_resource_pool.get(new_resource_handle);
		new_resource_data.info.id = resource_id;
		new_resource_data.info.type_id = type_id;
		new_resource_data.resource = nullptr;

		return new_resource_handle;
	}

	bool ResourceSystem::internal_create_resource(ResourceData& resource_data)
	{
		ResourceBase* new_resource_object = Vadon::Scene::ResourceRegistry::create_resource(resource_data.info.type_id);
		if (new_resource_object == nullptr)
		{
			// FIXME: show type name?
			log_error(std::format("Resource system error: failed to create resource with type ID {}!\n", Vadon::Utilities::to_integral(resource_data.info.type_id)));
			return false;
		}

		resource_data.resource = new_resource_object;
		return true;
	}

	bool ResourceSystem::has_resource(ResourceHandle resource_handle) const
	{
		return m_resource_pool.is_handle_valid(resource_handle);
	}
}