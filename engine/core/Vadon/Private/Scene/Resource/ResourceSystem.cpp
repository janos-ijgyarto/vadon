#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <format>

namespace
{
	void invalid_resource_data_error(const Vadon::Scene::ResourcePath path)
	{
		// TODO: print absolute path!
		Vadon::Core::Logger::log_error(std::format("Resource system error: invalid resource data in file \"{}\"!\n", path.path));
	}

	void resource_failed_to_serialize()
	{
		// TODO: print resource UUID!
		Vadon::Core::Logger::log_error("Resource system: failed to serialize resource data!\n");
	}
}

namespace Vadon::Private::Scene
{
	class ResourceSystem::LoadContext final : public Vadon::Scene::ResourceSystemInterface
	{
	public:
		LoadContext(Vadon::Private::Scene::ResourceSystem& system_impl, ResourceHandle loaded_resource_handle)
			: m_resource_system_impl(system_impl)
		{
			m_dependency_stack.push_back(loaded_resource_handle);
		}

		const ResourceBase* get_resource_base(ResourceHandle resource_handle) const override { return m_resource_system_impl.get_resource_base(resource_handle); }
		ResourceID get_resource_id(ResourceHandle resource_handle) const override { return m_resource_system_impl.get_resource_id(resource_handle); }
		Vadon::Utilities::TypeID get_resource_type_id(ResourceHandle resource_handle) const override { return m_resource_system_impl.get_resource_type_id(resource_handle); }

		ResourcePath get_resource_path(ResourceHandle resource_handle) const override { return m_resource_system_impl.get_resource_path(resource_handle); }
		void set_resource_path(ResourceHandle resource_handle, ResourcePath path) override { m_resource_system_impl.set_resource_path(resource_handle, path); }

		ResourceHandle find_resource(ResourceID resource_id) const override { return m_resource_system_impl.find_resource(resource_id); }
		bool load_resource(ResourceHandle resource_handle) override
		{
			// Make sure what we are trying to load isn't already in the stack (which would mean we have a circular dependency)
			if (std::find(m_dependency_stack.begin(), m_dependency_stack.end(), resource_handle) != m_dependency_stack.end())
			{
				// TODO: print resource info!
				Vadon::Core::Logger::log_error(std::format("Resource system: resource has circular dependency!\n"));
				return false;
			}

			// Add resource to stack before continuing the load
			m_dependency_stack.push_back(resource_handle);

			// Attempt to load
			const bool result = m_resource_system_impl.load_resource(*this, resource_handle);

			// Pop from stack
			m_dependency_stack.pop_back();

			return result;
		}

		std::vector<ResourceHandle> find_resources_of_type(Vadon::Utilities::TypeID type_id) const override { return m_resource_system_impl.find_resources_of_type(type_id); }

		ResourceHandle find_resource(ResourcePath resource_path) const override { return m_resource_system_impl.find_resource(resource_path); }
		ResourceHandle import_resource(ResourcePath resource_path) override { return m_resource_system_impl.import_resource(resource_path); }
		bool export_resource(ResourceHandle resource_handle) override { return m_resource_system_impl.export_resource(resource_handle); }

	protected:
		bool serialize_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, ResourceHandle& property_value) override { return m_resource_system_impl.serialize_resource_property(serializer, property_name, property_value); }
	protected:
		ResourceHandle internal_create_resource(ResourceBase* resource, Vadon::Utilities::TypeID type_id) override { return m_resource_system_impl.internal_create_resource(resource, type_id); }
	private:
		Vadon::Private::Scene::ResourceSystem& m_resource_system_impl;

		std::vector<ResourceHandle> m_dependency_stack;
	};

	const ResourceBase* ResourceSystem::get_resource_base(ResourceHandle resource_handle) const
	{
		return m_resource_pool.get(resource_handle).resource;
	}

	ResourceID ResourceSystem::get_resource_id(ResourceHandle resource_handle) const
	{
		const ResourceData& resource_data = m_resource_pool.get(resource_handle);
		return resource_data.id;
	}

	Vadon::Utilities::TypeID ResourceSystem::get_resource_type_id(ResourceHandle resource_handle) const
	{
		const ResourceData& resource_data = m_resource_pool.get(resource_handle);
		return resource_data.type_id;
	}

	ResourcePath ResourceSystem::get_resource_path(ResourceHandle resource_handle) const
	{
		const ResourceData& resource_data = m_resource_pool.get(resource_handle);
		return resource_data.path;
	}

	void ResourceSystem::set_resource_path(ResourceHandle resource_handle, ResourcePath path)
	{
		if (path.is_valid() == false)
		{
			Vadon::Core::Logger::log_error(std::format("Resource system error: cannot set invalid path for resource!\n"));
			return;
		}

		ResourceData& resource_data = m_resource_pool.get(resource_handle);
		resource_data.path = path;
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

	bool ResourceSystem::load_resource(ResourceHandle resource_handle)
	{
		return load_resource(*this, resource_handle);
	}

	std::vector<ResourceHandle> ResourceSystem::find_resources_of_type(Vadon::Utilities::TypeID type_id) const
	{
		std::vector<ResourceHandle> result;

		for (const auto& resource_obj_pair : m_resource_pool)
		{
			const ResourceData* current_resource_data = resource_obj_pair.second;
			if (Vadon::Utilities::TypeRegistry::is_base_of(type_id, current_resource_data->type_id) == true)
			{
				result.push_back(resource_obj_pair.first);
			}
		}

		return result;
	}

	ResourceHandle ResourceSystem::find_resource(ResourcePath resource_path) const
	{
		for (const auto& resource_obj_pair : m_resource_pool)
		{
			const ResourceData* current_resource_data = resource_obj_pair.second;
			if (current_resource_data->path == resource_path)
			{
				return resource_obj_pair.first;
			}
		}

		return ResourceHandle();
	}

	ResourceHandle ResourceSystem::import_resource(ResourcePath resource_path)
	{
		ResourceHandle imported_resource_handle;
		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();

		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
		if (file_system.load_file(Vadon::Core::FileSystem::Path{ .path = resource_path.path, .root = resource_path.root_directory }, resource_file_buffer) == false)
		{
			return imported_resource_handle;
		}

		// FIXME: determine whether it's binary or text and use appropriate serializer!
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);
		if (serializer->initialize() == false)
		{
			log_error(std::format("Resource system error: failed to load resource file \"{}\"!\n", resource_path.path));
			return imported_resource_handle;
		}

		// FIXME: should not have to load entire resource file just to register ID
		// FIXME2: use properties to serialize ResourceBase directly, which would allow deserializing a dummy object to get metadata
		// Need to allow TypeIDs to be serialized as string and deserialized to the ID number
		ResourceID resource_id;
		if (resource_id.serialize(*serializer, "id") == false)
		{
			invalid_resource_data_error(resource_path);
			return imported_resource_handle;
		}

		// Get type
		std::string resource_type;
		if (serializer->serialize("type", resource_type) == false)
		{
			invalid_resource_data_error(resource_path);
			return imported_resource_handle;
		}

		const Vadon::Utilities::TypeID resource_type_id = Vadon::Utilities::TypeRegistry::get_type_id(resource_type);
		if (resource_type_id == Vadon::Utilities::c_invalid_type_id)
		{
			log_error(std::format("Resource system error: resource file \"{}\" uses unknown type \"{}\"!\n", resource_path.path, resource_type));
			return imported_resource_handle;
		}

		// Add to lookup
		imported_resource_handle = find_resource(resource_id);
		if (imported_resource_handle.is_valid() == false)
		{
			imported_resource_handle = create_resource_data(resource_id, resource_type_id);

			ResourceData& new_resource_data = m_resource_pool.get(imported_resource_handle);
			new_resource_data.path = resource_path;
		}
		else
		{
			log_warning(std::format("Resource at path \"{}\" already imported!\n", resource_path.path));
		}

		return imported_resource_handle;
	}

	bool ResourceSystem::export_resource(ResourceHandle resource_handle)
	{
		ResourceData& resource_data = m_resource_pool.get(resource_handle);
		if (resource_data.is_loaded() == false)
		{
			log_error("Resource system error: attempting to export a resource that has not been loaded yet!\n");
			return false;
		}

		if (resource_data.path.is_valid() == false)
		{
			log_error("Resource system error: cannot export a resource without a valid path!\n");
			return false;
		}

		// Save the scene to JSON
		// TODO: allow deciding whether to use text or binary?
		Vadon::Core::FileSystem::RawFileDataBuffer resource_data_buffer;
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(resource_data_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::WRITE);
		if (serializer->initialize() == false)
		{
			return false;
		}

		// Save ID and type
		if (resource_data.id.serialize(*serializer, "id") == false)
		{
			resource_failed_to_serialize();
			return false;
		}

		{
			std::string resource_type = Vadon::Utilities::TypeRegistry::get_type_info(resource_data.type_id).name;
			if (serializer->serialize("type", resource_type) == false)
			{
				resource_failed_to_serialize();
				return false;
			}
		}

		if (serialize_resource_data(*this, *serializer, resource_data) == false)
		{
			return false;
		}

		if (serializer->finalize() == false)
		{
			resource_failed_to_serialize();
			return false;
		}

		if (m_engine_core.get_system<Vadon::Core::FileSystem>().save_file(Vadon::Core::FileSystem::Path{ .path = resource_data.path.path, .root = resource_data.path.root_directory }, resource_data_buffer) == false)
		{
			return false;
		}

		return true;
	}

	bool ResourceSystem::serialize_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, ResourceHandle& property_value)
	{
		if (serializer.is_reading() == true)
		{
			Vadon::Utilities::Variant resource_id_variant;
			if (serializer.serialize(property_name, resource_id_variant) == true)
			{
				const ResourceID resource_id = std::get<ResourceID>(resource_id_variant);
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
			Vadon::Utilities::Variant resource_id_variant = property_value.is_valid() ? get_resource_id(property_value) : ResourceID();
			if (serializer.serialize(property_name, resource_id_variant) == false)
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

		ResourceHandle new_resource_handle = create_resource_data(new_resource_id, type_id);

		ResourceData& new_resource_data = m_resource_pool.get(new_resource_handle);
		new_resource_data.resource = resource;

		return new_resource_handle;
	}

	ResourceSystem::ResourceSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Scene::ResourceSystem(core)
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
			if (current_resource_data->resource != nullptr)
			{
				delete current_resource_data->resource;
				current_resource_data->resource = nullptr;
			}
		}
		log_message("Resource System shut down!\n");
	}

	ResourceHandle ResourceSystem::create_resource_data(ResourceID resource_id, Vadon::Utilities::TypeID type_id)
	{
		// Create object, add to lookup
		ResourceHandle new_resource_handle = m_resource_pool.add();
		m_resource_lookup[resource_id] = new_resource_handle;

		// Set metadata
		ResourceData& new_resource_data = m_resource_pool.get(new_resource_handle);
		new_resource_data.id = resource_id;
		new_resource_data.type_id = type_id;

		return new_resource_handle;
	}

	bool ResourceSystem::load_resource(Vadon::Scene::ResourceSystemInterface& context, ResourceHandle resource_handle)
	{
		// First check whether it's loaded already
		ResourceData& resource_data = m_resource_pool.get(resource_handle);
		if (resource_data.is_loaded() == true)
		{
			return true;
		}

		// Resource not yet loaded
		if (&context == this)
		{
			LoadContext load_context(*this, resource_handle);
			return internal_load_resource(load_context, resource_data);
		}
		else
		{
			return internal_load_resource(context, resource_data);
		}
	}

	bool ResourceSystem::internal_load_resource(Vadon::Scene::ResourceSystemInterface& context, ResourceData& resource_data)
	{
		if (resource_data.path.is_valid() == false)
		{
			// TODO: print resource ID!
			log_error("Resource system error: unable to load resource with invalid path!\n");
			return false;
		}

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();

		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
		if (file_system.load_file(Vadon::Core::FileSystem::Path{ .path = resource_data.path.path, .root = resource_data.path.root_directory }, resource_file_buffer) == false)
		{
			return false;
		}

		// FIXME: determine whether it's binary or text and use appropriate serializer!
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);
		if (serializer->initialize() == false)
		{
			log_error(std::format("Resource system error: failed to load resource file \"{}\"!\n", resource_data.path.path));
			return false;
		}

		// TODO: check to make sure ID and type matches?

		// Create new resource
		resource_data.resource = Vadon::Scene::ResourceRegistry::create_resource(resource_data.type_id);
		if (resource_data.resource == nullptr)
		{
			// FIXME: show type name?
			log_error(std::format("Resource system error: failed to create resource with type ID {}!\n", resource_data.type_id));
			return false;
		}

		if (serialize_resource_data(context, *serializer, resource_data) == false)
		{
			delete resource_data.resource;
			return false;
		}

		return true;
	}

	bool ResourceSystem::serialize_resource_data(Vadon::Scene::ResourceSystemInterface& context, Vadon::Utilities::Serializer& serializer, const ResourceData& resource_data)
	{
		Vadon::Scene::ResourceRegistry::SerializerFunction serializer_function = Vadon::Scene::ResourceRegistry::get_serializer(resource_data.type_id);
		if (serializer.is_reading() == true)
		{
			// Get list of properties to load (if using custom serializer, only load the base properties)
			const Vadon::Utilities::PropertyInfoList resource_properties = (serializer_function == nullptr) ?
				Vadon::Utilities::TypeRegistry::get_type_properties(resource_data.type_id) : Vadon::Utilities::TypeRegistry::get_type_properties<ResourceBase>();

			for (const Vadon::Utilities::PropertyInfo& current_property_info : resource_properties)
			{
				Vadon::Utilities::Variant current_property_value;
				if (serializer.serialize(current_property_info.name, current_property_value) == true)
				{
					Vadon::Utilities::TypeRegistry::set_property(resource_data.resource, resource_data.type_id, current_property_info.name, current_property_value);
				}
				else
				{
					invalid_resource_data_error(resource_data.path);
					return false;
				}
			}
		}
		else
		{
			// Get list of properties to save (if using custom serializer, only save the base properties)
			Vadon::Utilities::PropertyList resource_properties = (serializer_function == nullptr) ?
				Vadon::Utilities::TypeRegistry::get_properties(resource_data.resource, resource_data.type_id) : Vadon::Utilities::TypeRegistry::get_properties(resource_data.resource, Vadon::Utilities::TypeRegistry::get_type_id<ResourceBase>());

			for (Vadon::Utilities::Property& current_property : resource_properties)
			{
				if (serializer.serialize(current_property.name, current_property.value) == false)
				{
					invalid_resource_data_error(resource_data.path);
					return false;
				}
			}
		}

		if (serializer_function != nullptr)
		{
			// Use the custom serializer
			if (serializer_function(context, serializer, *resource_data.resource) == false)
			{
				log_error(std::format("Resource system error: failed to load resource from file \"{}\"!\n", resource_data.path.path));
				return false;
			}
		}

		return true;
	}
}