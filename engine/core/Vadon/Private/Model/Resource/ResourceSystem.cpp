#include <Vadon/Private/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Model/Resource/File.hpp>
#include <Vadon/Model/Resource/Registry.hpp>
#include <Vadon/Model/Resource/Database.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <Vadon/Utilities/Enum/EnumClass.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/PropertySerialization.hpp>

#include <Vadon/Foundation/Model/Resource/Resource.hpp>
#include <Vadon/Foundation/TypeInfo/Metadata.hpp>

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

		static constexpr auto c_process_trivial_property = +[](Vadon::Utilities::Serializer& serializer, size_t index, Vadon::Utilities::TypeID data_type, Vadon::Utilities::Variant& value)
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
		const Vadon::Utilities::TypeID erased_array_data_type = Vadon::Utilities::get_erased_data_type_id(array.data_type);

		for (size_t index = 0; index < array.data.size(); ++index)
		{
			Vadon::Utilities::Variant& current_value = array.data[index];
			successful &= c_process_trivial_property(serializer, index, erased_array_data_type, current_value);
		}

		return successful;
	}

	bool load_resource_array_data(Vadon::Utilities::Serializer& serializer, Vadon::Utilities::Variant& array_value)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;

		Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(array_value);
		array.data.clear();

		static constexpr auto c_process_trivial_property = +[](Vadon::Utilities::Serializer& serializer, size_t index, Vadon::Utilities::TypeID data_type, Vadon::Utilities::Variant& value)
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
		const Vadon::Utilities::TypeID erased_array_data_type = Vadon::Utilities::get_erased_data_type_id(array.data_type);
		for (size_t index = 0; index < array_size; ++index)
		{
			successful &= c_process_trivial_property(serializer, index, erased_array_data_type, current_value);
			array.data.push_back(current_value);
		}
		
		return successful;
	}
}

namespace Vadon::Private::Model
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
			new_resource_id = Vadon::Utilities::generate_uuid();
			if (find_resource(new_resource_id).is_valid() == false)
			{
				break;
			}
		}

		return internal_add_resource(ResourceInfo{ .id = new_resource_id, .type_id = type_id }, resource);
	}

	void ResourceSystem::add_embedded_resource(ResourceHandle owner_handle, ResourceHandle embedded_resource_handle)
	{
		// TODO: check to make sure we didn't create a circular embedding
		ResourceData& embedded_resource_data = m_resource_pool.get(embedded_resource_handle);
		VADON_ASSERT(embedded_resource_data.owner.is_valid() == false, "Cannot embed resource that is already embedded!");

		embedded_resource_data.owner = owner_handle;

		ResourceData& owner_data = m_resource_pool.get(owner_handle);
		owner_data.embedded_resources.push_back(embedded_resource_handle);
	}

	ResourceHandle ResourceSystem::get_embedded_resource_onwer(ResourceHandle resource_handle) const
	{
		const ResourceData& resource_data = m_resource_pool.get(resource_handle);
		return resource_data.owner;
	}

	std::vector<ResourceHandle> ResourceSystem::get_embedded_resources(ResourceHandle resource_handle) const
	{
		const ResourceData& resource_data = m_resource_pool.get(resource_handle);
		return resource_data.embedded_resources;
	}

	void ResourceSystem::remove_embedded_resource(ResourceHandle owner_handle, ResourceHandle embedded_resource_handle)
	{
		ResourceData& embedded_resource_data = m_resource_pool.get(embedded_resource_handle);
		VADON_ASSERT(embedded_resource_data.owner.is_valid() == true, "Must be an embedded resource!");
		VADON_ASSERT(embedded_resource_data.owner == owner_handle, "Owner handle does not match!");

		embedded_resource_data.owner.invalidate();

		// Remove from owner
		ResourceData& owner_data = m_resource_pool.get(owner_handle);
		for (size_t embedded_resource_index = 0; embedded_resource_index < owner_data.embedded_resources.size(); ++embedded_resource_index)
		{
			if (owner_data.embedded_resources[embedded_resource_index] == embedded_resource_handle)
			{
				owner_data.embedded_resources.erase(owner_data.embedded_resources.begin() + embedded_resource_index);
				break;
			}
		}

		// Now we can remove the resource itself
		remove_resource(embedded_resource_handle);
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
		if (serializer.serialize(Vadon::Utilities::Property::property_schema_to_uuid(::Vadon::Foundation::ResourceSchema::c_id_property), resource_info.id) != SerializerResult::SUCCESSFUL)
		{
			resource_info_failed_to_serialize();
			return false;
		}

		::Vadon::Foundation::UUID resource_type_uuid;
		if (serializer.serialize(Vadon::Utilities::Property::property_schema_to_uuid(::Vadon::Foundation::ResourceSchema::c_type_property), resource_type_uuid) != SerializerResult::SUCCESSFUL)
		{
			resource_info_failed_to_serialize();
			return false;
		}

		resource_info.type_id = Vadon::Utilities::TypeRegistry::get_type_id(resource_type_uuid);
		if (resource_info.type_id == Vadon::Utilities::TypeID::INVALID)
		{
			log_error(std::format("Resource system error: resource data uses unknown type \"{}\"!\n", Vadon::Utilities::uuid_to_string(resource_type_uuid)));
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

		ResourceData& resource_data = m_resource_pool.get(resource_handle);

		if (serializer.serialize(Vadon::Utilities::Property::property_schema_to_uuid(::Vadon::Foundation::ResourceSchema::c_id_property), resource_data.info.id) != SerializerResult::SUCCESSFUL)
		{
			resource_info_failed_to_serialize();
			return false;
		}

		// Get type
		::Vadon::Foundation::UUID resource_type_uuid = Vadon::Utilities::TypeRegistry::get_type_info(resource_data.info.type_id).id;
		if (serializer.serialize(Vadon::Utilities::Property::property_schema_to_uuid(::Vadon::Foundation::ResourceSchema::c_type_property), resource_type_uuid) != SerializerResult::SUCCESSFUL)
		{
			resource_info_failed_to_serialize();
			return false;
		}

		// Serialize properties
		if (serializer.open_object(Vadon::Utilities::Property::property_schema_to_uuid(::Vadon::Foundation::ResourceSchema::c_properties_property)) != SerializerResult::SUCCESSFUL)
		{
			resource_data_failed_to_serialize();
			return false;
		}

		static constexpr auto c_process_trivial_property = +[](Vadon::Utilities::Serializer& serializer, const Vadon::Utilities::PropertyUUID& key, Vadon::Utilities::TypeID data_type, Vadon::Utilities::Variant& value)
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
			const Vadon::Utilities::PropertyInfo property_info = Vadon::Utilities::TypeRegistry::get_property_info(resource_data.info.type_id, current_property.info.id);
			const Vadon::Utilities::TypeID property_type_id = Vadon::Utilities::TypeRegistry::get_type_id(property_info.base_info.type);

			if (property_type_id == Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Utilities::BoxedVariantArray>())
			{
				if (serializer.open_array(property_info.base_info.type) != SerializerResult::SUCCESSFUL)
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
			else
			{
				c_process_trivial_property(serializer, current_property.info.id, property_type_id, current_property.value);
			}
		}

		if (serializer.close_object() != SerializerResult::SUCCESSFUL)
		{
			resource_data_failed_to_serialize();
			return false;
		}

		Vadon::Model::ResourceRegistry::SerializerFunction resource_serializer = nullptr;//Vadon::Model::ResourceRegistry::get_resource_serializer(resource_data.info.type_id);
		if (resource_serializer != nullptr)
		{
			if (serializer.open_object(Vadon::Utilities::Property::property_schema_to_uuid(::Vadon::Foundation::ResourceSchema::c_data_property)) != SerializerResult::SUCCESSFUL)
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

		if (resource_data.embedded_resources.empty() == false)
		{
			if (serializer.open_array(Vadon::Utilities::Property::property_schema_to_uuid(::Vadon::Foundation::ResourceSchema::c_embedded_property)) != SerializerResult::SUCCESSFUL)
			{
				resource_data_failed_to_serialize();
				return false;
			}

			for (size_t embedded_resource_index = 0; embedded_resource_index < resource_data.embedded_resources.size(); ++embedded_resource_index)
			{
				if (serializer.open_object(embedded_resource_index) != SerializerResult::SUCCESSFUL)
				{
					resource_data_failed_to_serialize();
					return false;
				}

				if (save_resource(serializer, resource_data.embedded_resources[embedded_resource_index]) == false)
				{
					resource_data_failed_to_serialize();
					return false;
				}

				if (serializer.close_object() != SerializerResult::SUCCESSFUL)
				{
					resource_data_failed_to_serialize();
					return false;
				}
			}

			if (serializer.close_array() != SerializerResult::SUCCESSFUL)
			{
				resource_data_failed_to_serialize();
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
		std::vector<ResourceHandle> embedded_resources;
		Resource* resource_data = load_resource_data(serializer, embedded_resources, resource_info);
		if (resource_data == nullptr)
		{
			log_error("Resource system: failed to deserialize Resource data!\n");
			return ResourceHandle();
		}

		// Everything succeeded, add to pool
		loaded_resource_handle = internal_add_resource(resource_info, resource_data);

		// Add embedded resources
		for (ResourceHandle current_embedded_resource : embedded_resources)
		{
			add_embedded_resource(loaded_resource_handle, current_embedded_resource);
		}

		return loaded_resource_handle;
	}

	void ResourceSystem::remove_resource(ResourceHandle resource_handle)
	{
		ResourceData& resource_data = m_resource_pool.get(resource_handle);

		if (resource_data.owner.is_valid() == true)
		{
			VADON_ERROR("Embedded resource must be removed explicitly from owner!");
			return;
		}

		// Remove from lookup
		m_resource_lookup.erase(resource_data.info.id);

		// Delete resource data
		delete resource_data.resource;
		resource_data.resource = nullptr;

		// Remove embedded resources
		for (const ResourceHandle& embedded_resource_handle : resource_data.embedded_resources)
		{
			// Invalidate self in embedded resource (allows them to be removed)
			ResourceData& embedded_resource_data = m_resource_pool.get(embedded_resource_handle);
			VADON_ASSERT(embedded_resource_data.owner == resource_handle, "Embedded resource owner mismatch!");
			embedded_resource_data.owner.invalidate();

			// Remove the embedded resource as well
			remove_resource(embedded_resource_handle);
		}

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
		: Vadon::Model::ResourceSystem(core)
	{}

	void ResourceSystem::register_types()
	{
		Vadon::Model::ResourceRegistry::register_resource_type<Resource>();
		Vadon::Utilities::TypeRegistry::add_property<Resource>(VADON_GET_MEMBER_UUID(Resource, name),
			Vadon::Utilities::create_member_variable_bind<Resource, &Resource::name>().bind_member_getter().bind_member_setter());

		Vadon::Model::ResourceRegistry::register_resource_type<Vadon::Model::FileResource, Vadon::Model::Resource>();
	}

	void ResourceSystem::register_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::Utilities::TypeMetadata resource_metadata(metadata_registry, VADON_GET_TYPE_UUID(Vadon::Model::Resource));

		resource_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Scene::Resource");

		Vadon::Utilities::TypePropertyMetadata name_property(resource_metadata, VADON_GET_MEMBER_UUID(Vadon::Model::Resource, name));
		name_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Name");

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(Vadon::Model::FileResource))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Scene::FileResource");
	}

	bool ResourceSystem::initialize()
	{
		using Vadon::Utilities::operator""_uuid;

		log_message("Initializing Resource System\n");
		// TODO: anything?
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

	Resource* ResourceSystem::load_resource_data(Vadon::Utilities::Serializer& serializer, std::vector<ResourceHandle>& embedded_resources, const ResourceInfo& info)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;

		// Create resource object
		std::unique_ptr<Resource> resource(internal_create_resource(info.type_id));
		if (resource == nullptr)
		{
			return nullptr;
		}

		// Deserialize resource data
		if (serializer.open_object(Vadon::Utilities::Property::property_schema_to_uuid(::Vadon::Foundation::ResourceSchema::c_properties_property)) != SerializerResult::SUCCESSFUL)
		{
			resource_data_failed_to_serialize();
			return nullptr;
		}

		static constexpr auto c_process_trivial_property = +[](Vadon::Utilities::Serializer& serializer, Resource* resource_ptr, const ResourceInfo& info, const Vadon::Utilities::PropertyUUID& key, Vadon::Utilities::TypeID data_type, Vadon::Utilities::Variant& value)
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
			// If key is not present, just use default value
			if (serializer.has_key(current_property_info.base_info.id) == false)
			{
				log_warning(std::format("Resource system: cannot find property \"{}\" in resource \"{}\"!\n", Vadon::Utilities::uuid_to_string(current_property_info.base_info.id), Vadon::Utilities::uuid_to_string(info.id)));
				continue;
			}

			const Vadon::Utilities::PropertyInfo property_info = Vadon::Utilities::TypeRegistry::get_property_info(info.type_id, current_property_info.base_info.id);
			const Vadon::Utilities::TypeID property_type_id = Vadon::Utilities::TypeRegistry::get_type_id(property_info.base_info.type);

			if (property_type_id == Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Utilities::BoxedVariantArray>())
			{
				if (serializer.open_array(current_property_info.base_info.id) != SerializerResult::SUCCESSFUL)
				{
					resource_data_failed_to_serialize();
					return nullptr;
				}

				// Get array property (to retrieve array member type info)
				current_property_value = Vadon::Utilities::TypeRegistry::get_property(resource.get(), info.type_id, current_property_info.base_info.id);
				if (load_resource_array_data(serializer, current_property_value) == true)
				{
					// Successfully loaded, write back into object
					Vadon::Utilities::TypeRegistry::set_property(resource.get(), info.type_id, current_property_info.base_info.id, current_property_value);
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
			else
			{
				c_process_trivial_property(serializer, resource.get(), info, current_property_info.base_info.id, property_type_id, current_property_value);
			}
		}

		if (serializer.close_object() != SerializerResult::SUCCESSFUL)
		{
			resource_data_failed_to_serialize();
			return nullptr;
		}

		Vadon::Model::ResourceRegistry::SerializerFunction resource_serializer = nullptr; //Vadon::Model::ResourceRegistry::get_resource_serializer(info.type_id);
		if (resource_serializer != nullptr)
		{
			if (serializer.open_object(Vadon::Utilities::Property::property_schema_to_uuid(::Vadon::Foundation::ResourceSchema::c_data_property)) != SerializerResult::SUCCESSFUL)
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

		constexpr auto c_embedded_property_uuid = Vadon::Utilities::Property::property_schema_to_uuid(::Vadon::Foundation::ResourceSchema::c_embedded_property);
		if (serializer.has_key(c_embedded_property_uuid) == true)
		{
			if (serializer.open_array(c_embedded_property_uuid) != SerializerResult::SUCCESSFUL)
			{
				resource_custom_data_failed_to_serialize();
				return nullptr;
			}
			const size_t embedded_resource_count = serializer.get_array_size();
			for (size_t embedded_resource_index = 0; embedded_resource_index < embedded_resource_count; ++embedded_resource_index)
			{
				if (serializer.open_object(embedded_resource_index) != SerializerResult::SUCCESSFUL)
				{
					resource_data_failed_to_serialize();
					return nullptr;
				}

				ResourceHandle embedded_resource_handle = load_resource(serializer);
				if (embedded_resource_handle.is_valid() == false)
				{
					resource_data_failed_to_serialize();
					return nullptr;
				}

				embedded_resources.push_back(embedded_resource_handle);

				if (serializer.close_object() != SerializerResult::SUCCESSFUL)
				{
					resource_data_failed_to_serialize();
					return nullptr;
				}
			}
			if (serializer.close_array() != SerializerResult::SUCCESSFUL)
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
		Resource* new_resource = Vadon::Model::ResourceRegistry::create_resource(type_id);
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