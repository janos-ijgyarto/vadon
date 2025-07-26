#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Scene/Resource/File.hpp>
#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Scene/Resource/Database.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <Vadon/Utilities/Enum/EnumClass.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/PropertySerialization.hpp>

#include <format>

namespace
{
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

	bool save_resource_array_data(Vadon::Utilities::Serializer& serializer, Vadon::Utilities::Variant& array_value)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;
		using ErasedDataType = Vadon::Utilities::ErasedDataType;

		static constexpr auto c_process_trivial_property = +[](Vadon::Utilities::Serializer& serializer, size_t index, Vadon::Utilities::ErasedDataTypeID data_type, Vadon::Utilities::Variant& value)
			{
				const SerializerResult result = Vadon::Utilities::process_trivial_property(serializer, index, value, data_type);
				if (result != SerializerResult::SUCCESSFUL)
				{
					invalid_resource_data_error();
					return false;
				}

				return true;
			};

		bool successful = true;

		Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(array_value);
		for (size_t index = 0; index < array.data.size(); ++index)
		{
			Vadon::Utilities::Variant& current_value = array.data[index];
			switch (array.data_type.type)
			{
			case ErasedDataType::TRIVIAL:
				successful &= c_process_trivial_property(serializer, index, array.data_type, current_value);
				break;
			case ErasedDataType::RESOURCE_ID:
				successful &= c_process_trivial_property(serializer, index, Vadon::Utilities::get_erased_data_type_id<Vadon::Utilities::UUID>(), current_value);
				break;
			default:
				VADON_UNREACHABLE;
				break;
			}
		}

		return successful;
	}

	bool load_resource_array_data(Vadon::Utilities::Serializer& serializer, Vadon::Utilities::Variant& array_value)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;
		using ErasedDataType = Vadon::Utilities::ErasedDataType;

		Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(array_value);
		array.data.clear();

		static constexpr auto c_process_trivial_property = +[](Vadon::Utilities::Serializer& serializer, size_t index, Vadon::Utilities::ErasedDataTypeID data_type, Vadon::Utilities::Variant& value)
			{
				const SerializerResult result = process_trivial_property(serializer, index, value, data_type);
				if (result != SerializerResult::SUCCESSFUL)
				{
					invalid_resource_data_error();
					return false;
				}

				return true;
			};

		bool successful = true;

		Vadon::Utilities::Variant current_value;
		const size_t array_size = serializer.get_array_size();
		for (size_t index = 0; index < array_size; ++index)
		{
			switch (array.data_type.type)
			{
			case ErasedDataType::TRIVIAL:
				successful &= c_process_trivial_property(serializer, index, array.data_type, current_value);
				break;
			case ErasedDataType::RESOURCE_ID:
				successful &= c_process_trivial_property(serializer, index, Vadon::Utilities::get_erased_data_type_id<Vadon::Utilities::UUID>(), current_value);
				break;
			default:
				VADON_UNREACHABLE;
				break;
			}

			array.data.push_back(current_value);
		}
		
		return successful;
	}
}

namespace Vadon::Private::Scene
{
	ResourceHandle ResourceSystem::create_resource(Vadon::Utilities::TypeID type_id)
	{
		Resource* resource = internal_create_resource(type_id);
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

	void ResourceSystem::register_database(ResourceDatabase& database)
	{
		m_database_list.push_back(&database);
	}

	bool ResourceSystem::load_resource_info(Vadon::Utilities::Serializer& serializer, ResourceInfo& resource_info) const
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;

		if (serializer.is_reading() == false)
		{
			log_message("Resource system: invalid serializer!\n");
			return false;
		}

		ResourceID resource_id;
		if (serializer.serialize("id", resource_info.id) != SerializerResult::SUCCESSFUL)
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

		resource_info.type_id = Vadon::Utilities::TypeRegistry::get_type_id(resource_type);
		if (resource_info.type_id == Vadon::Utilities::TypeID::INVALID)
		{
			log_error(std::format("Resource system error: resource data uses unknown type \"{}\"!\n", resource_type));
			return false;
		}

		return true;
	}

	bool ResourceSystem::save_resource(ResourceHandle resource_handle)
	{
		for (ResourceDatabase* current_database : m_database_list)
		{
			if(current_database->save_resource(*this, resource_handle) == true)
			{
				return true;
			}
		}

		return false;
	}

	ResourceHandle ResourceSystem::load_resource_base(ResourceID resource_id)
	{
		// Check whether it's already loaded
		ResourceHandle result = find_resource(resource_id);
		if (result.is_valid() == true)
		{
			return result;
		}

		for (ResourceDatabase* current_database : m_database_list)
		{
			result = current_database->load_resource(*this, resource_id);
			if (result.is_valid() == true)
			{
				break;
			}
		}

		return result;
	}

	bool ResourceSystem::save_resource(Vadon::Utilities::Serializer& serializer, ResourceHandle resource_handle)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;
		using ErasedDataType = Vadon::Utilities::ErasedDataType;

		ResourceData& resource_data = m_resource_pool.get(resource_handle);

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

		static constexpr auto c_process_trivial_property = +[](Vadon::Utilities::Serializer& serializer, std::string_view key, Vadon::Utilities::ErasedDataTypeID data_type, Vadon::Utilities::Variant& value)
			{
				const SerializerResult result = Vadon::Utilities::process_trivial_property(serializer, key, value, data_type);
				if (result != SerializerResult::SUCCESSFUL)
				{
					invalid_resource_data_error();
				}
			};

		Vadon::Utilities::PropertyList resource_properties = Vadon::Utilities::TypeRegistry::get_properties(resource_data.resource, resource_data.info.type_id);

		for (Vadon::Utilities::Property& current_property : resource_properties)
		{
			switch (current_property.data_type.type)
			{
			case ErasedDataType::TRIVIAL:
				c_process_trivial_property(serializer, current_property.name, current_property.data_type, current_property.value);
				break;
			case ErasedDataType::RESOURCE_ID:
				c_process_trivial_property(serializer, current_property.name, Vadon::Utilities::get_erased_data_type_id<Vadon::Utilities::UUID>(), current_property.value);
				break;
			case ErasedDataType::ARRAY:
			{
				if (serializer.open_array(current_property.name) != SerializerResult::SUCCESSFUL)
				{
					resource_data_failed_to_serialize();
					return false;
				}

				if (save_resource_array_data(serializer, current_property.value) == false)
				{
					// TODO: error?
				}

				if (serializer.close_array() != SerializerResult::SUCCESSFUL)
				{
					resource_data_failed_to_serialize();
					return false;
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

	ResourceHandle ResourceSystem::load_resource(Vadon::Utilities::Serializer& serializer)
	{
		ResourceInfo resource_info;
		if (load_resource_info(serializer, resource_info) == false)
		{
			log_error("Resource system: failed to deserialize Resource info!\n");
			return ResourceHandle();
		}

		// TODO: allow force-reloading?
		ResourceHandle loaded_resource_handle = find_resource(resource_info.id);
		if (loaded_resource_handle.is_valid() == true)
		{
			// Resource already loaded
			// TODO: notification?
			return loaded_resource_handle;
		}

		// Attempt to load Resource data
		Resource* resource_data = load_resource_data(serializer, resource_info);
		if (resource_data == nullptr)
		{
			log_error("Resource system: failed to deserialize Resource data!\n");
			return ResourceHandle();
		}

		// Everything succeeded, add to pool
		return internal_add_resource(resource_info, resource_data);
	}

	void ResourceSystem::remove_resource(ResourceHandle resource_handle)
	{
		ResourceData& resource_data = m_resource_pool.get(resource_handle);

		// Remove from lookup
		m_resource_lookup.erase(resource_data.info.id);

		// Delete resource data
		delete resource_data.resource;
		resource_data.resource = nullptr;

		m_resource_pool.remove(resource_handle);
	}

	const Resource* ResourceSystem::get_base_resource(ResourceHandle resource_handle) const
	{
		return m_resource_pool.get(resource_handle).resource;
	}

	Vadon::Core::FileInfo ResourceSystem::get_file_resource_info(ResourceID resource_id) const
	{
		for (ResourceDatabase* current_database : m_database_list)
		{
			Vadon::Core::FileInfo file_info = current_database->get_file_resource_info(resource_id);
			if (file_info.is_valid() == true)
			{
				return file_info;
			}
		}

		return Vadon::Core::FileInfo();
	}

	bool ResourceSystem::load_file_resource_data(ResourceID resource_id, Vadon::Core::RawFileDataBuffer& file_data)
	{
		for (ResourceDatabase* current_database : m_database_list)
		{
			if(current_database->load_file_resource_data(*this, resource_id, file_data) == true)
			{
				return true;
			}
		}

		return false;
	}

	ResourceSystem::ResourceSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Scene::ResourceSystem(core)
	{}

	bool ResourceSystem::initialize()
	{
		log_message("Initializing Resource System\n");

		Vadon::Scene::ResourceRegistry::register_resource_type<Resource>();
		Vadon::Utilities::TypeRegistry::add_property<Resource>("name", Vadon::Utilities::MemberVariableBind<&Resource::name>().bind_member_getter().bind_member_setter());

		Vadon::Scene::ResourceRegistry::register_resource_type<Vadon::Scene::FileResource, Vadon::Scene::Resource>();

		log_message("Resource System initialized successfully!\n");
		return true;
	}

	void ResourceSystem::shutdown()
	{
		log_message("Shutting down Resource System\n");
		for (auto resource_pair : m_resource_pool)
		{
			ResourceData* current_resource_data = resource_pair.second;

			delete current_resource_data->resource;
			current_resource_data->resource = nullptr;
		}
		log_message("Resource System shut down!\n");
	}

	Resource* ResourceSystem::load_resource_data(Vadon::Utilities::Serializer& serializer, const ResourceInfo& info)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;
		using ErasedDataType = Vadon::Utilities::ErasedDataType;

		// Create resource object
		std::unique_ptr<Resource> resource(internal_create_resource(info.type_id));
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

		static constexpr auto c_process_trivial_property = +[](Vadon::Utilities::Serializer& serializer, Resource* resource_ptr, const ResourceInfo& info, std::string_view key, Vadon::Utilities::ErasedDataTypeID data_type, Vadon::Utilities::Variant& value)
			{
				const SerializerResult result = Vadon::Utilities::process_trivial_property(serializer, key, value, data_type);
				if (result == SerializerResult::SUCCESSFUL)
				{
					Vadon::Utilities::TypeRegistry::set_property(resource_ptr, info.type_id, key, value);
				}
				else
				{
					invalid_resource_data_error();
				}
			}; 
		
		const Vadon::Utilities::PropertyInfoList resource_properties = Vadon::Utilities::TypeRegistry::get_type_properties(info.type_id);
		Vadon::Utilities::Variant current_property_value;
		for (const Vadon::Utilities::PropertyInfo& current_property_info : resource_properties)
		{
			switch (current_property_info.data_type.type)
			{
			case ErasedDataType::TRIVIAL:
				c_process_trivial_property(serializer, resource.get(), info, current_property_info.name, current_property_info.data_type, current_property_value);
				break;
			case ErasedDataType::RESOURCE_ID:
				c_process_trivial_property(serializer, resource.get(), info, current_property_info.name, Vadon::Utilities::get_erased_data_type_id<Vadon::Utilities::UUID>(), current_property_value);
				break;
			case ErasedDataType::ARRAY:
			{
				if (serializer.open_array(current_property_info.name) != SerializerResult::SUCCESSFUL)
				{
					resource_data_failed_to_serialize();
					return nullptr;
				}

				// Get array property (to retrieve array member type info)
				current_property_value = Vadon::Utilities::TypeRegistry::get_property(resource.get(), info.type_id, current_property_info.name);
				if (load_resource_array_data(serializer, current_property_value) == true)
				{
					// Successfully loaded, write back into object
					Vadon::Utilities::TypeRegistry::set_property(resource.get(), info.type_id, current_property_info.name, current_property_value);
				}
				else
				{
					// TODO: error?
				}

				if (serializer.close_array() != SerializerResult::SUCCESSFUL)
				{
					resource_data_failed_to_serialize();
					return nullptr;
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

	Resource* ResourceSystem::internal_create_resource(Vadon::Utilities::TypeID type_id) const
	{
		Resource* new_resource = Vadon::Scene::ResourceRegistry::create_resource(type_id);
		if (new_resource == nullptr)
		{
			// FIXME: show type name?
			log_error(std::format("Resource system error: failed to create resource with type ID {}!\n", Vadon::Utilities::to_integral(type_id)));
		}

		return new_resource;
	}

	ResourceHandle ResourceSystem::internal_add_resource(const ResourceInfo& info, Resource* resource)
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