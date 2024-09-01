#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <format>

namespace
{
	void resource_not_found_error(Vadon::Scene::ResourceID /*resource_id*/)
	{
		// FIXME: print resource UUID!
		Vadon::Core::Logger::log_error(std::format("Resource system error: resource with ID not found!\n"));
	}

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
		LoadContext(Vadon::Private::Scene::ResourceSystem& system_impl, ResourceID loaded_resource_id)
			: m_resource_system_impl(system_impl)
		{
			m_dependency_stack.push_back(loaded_resource_id);
		}

		const ResourceBase* get_resource_base(ResourceHandle resource_handle) const override { return m_resource_system_impl.get_resource_base(resource_handle); }
		ResourceID get_resource_id(ResourceHandle resource_handle) const override { return m_resource_system_impl.get_resource_id(resource_handle); }

		ResourcePath get_resource_path(ResourceID resource_id) const override { return m_resource_system_impl.get_resource_path(resource_id); }
		void set_resource_path(ResourceID resource_id, ResourcePath path) override { m_resource_system_impl.set_resource_path(resource_id, path); }

		ResourceHandle find_resource(ResourceID resource_id) const override { return m_resource_system_impl.find_resource(resource_id); }
		ResourceHandle load_resource(ResourceID resource_id) override 
		{
			// Make sure what we are trying to load isn't already in the stack (which would mean we have a circular dependency)
			if (std::find(m_dependency_stack.begin(), m_dependency_stack.end(), resource_id) != m_dependency_stack.end())
			{
				// TODO: print resource info!
				Vadon::Core::Logger::log_error(std::format("Resource system: resource has circular dependency!\n"));
				return ResourceHandle();
			}

			// Add resource to stack before continuing the load
			m_dependency_stack.push_back(resource_id);

			// Attempt to load
			const ResourceHandle result = m_resource_system_impl.load_resource(*this, resource_id);

			// Pop from stack
			m_dependency_stack.pop_back();

			return result;
		}

		std::vector<ResourceID> find_resources_of_type(Vadon::Utilities::TypeID type_id) const override { return m_resource_system_impl.find_resources_of_type(type_id); }

		ResourceID find_resource(ResourcePath resource_path) const override { return m_resource_system_impl.find_resource(resource_path); }
		ResourceID import_resource(ResourcePath resource_path) override { return m_resource_system_impl.import_resource(resource_path); }
		bool export_resource(ResourceID resource_id) override { return m_resource_system_impl.export_resource(resource_id); }

		bool serialize_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value) override { return m_resource_system_impl.serialize_resource_property(serializer, property_name, property_value); }
	protected:
		ResourceHandle internal_create_resource(ResourceBase* resource, Vadon::Utilities::TypeID type_id) override { return m_resource_system_impl.internal_create_resource(resource, type_id); }
	private:
		Vadon::Private::Scene::ResourceSystem& m_resource_system_impl;

		std::vector<ResourceID> m_dependency_stack;
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

	ResourcePath ResourceSystem::get_resource_path(ResourceID resource_id) const
	{
		const ResourceInfo* resource_info = internal_find_resource(resource_id);
		if (resource_info == nullptr)
		{
			resource_not_found_error(resource_id);
			return ResourcePath();
		}

		return resource_info->path;
	}

	void ResourceSystem::set_resource_path(ResourceID resource_id, ResourcePath path)
	{
		if (path.is_valid() == false)
		{
			Vadon::Core::Logger::log_error(std::format("Resource system error: cannot set invalid path for resource!\n"));
			return;
		}

		ResourceInfo* resource_info = internal_find_resource(resource_id);
		if (resource_info == nullptr)
		{
			resource_not_found_error(resource_id);
			return;
		}

		resource_info->path = path;
	}

	ResourceHandle ResourceSystem::find_resource(ResourceID resource_id) const
	{
		const ResourceInfo* resource_info = internal_find_resource(resource_id);
		if (resource_info == nullptr)
		{
			return ResourceHandle();
		}

		return resource_info->handle;
	}

	ResourceHandle ResourceSystem::load_resource(ResourceID resource_id)
	{
		return load_resource(*this, resource_id);
	}

	std::vector<ResourceID> ResourceSystem::find_resources_of_type(Vadon::Utilities::TypeID type_id) const
	{
		std::vector<ResourceID> result;

		for (auto& resource_info_pair : m_resource_lookup)
		{
			const ResourceInfo& resource_info = resource_info_pair.second;
			if (Vadon::Utilities::TypeRegistry::is_base_of(type_id, resource_info.type_id) == true)
			{
				result.push_back(resource_info_pair.first);
			}
		}

		return result;
	}

	ResourceID ResourceSystem::find_resource(ResourcePath resource_path) const
	{
		for (auto& resource_info_pair : m_resource_lookup)
		{
			const ResourceInfo& current_resource_info = resource_info_pair.second;
			if (current_resource_info.path == resource_path)
			{
				return resource_info_pair.first;
			}
		}

		return ResourceID();
	}

	ResourceID ResourceSystem::import_resource(ResourcePath resource_path)
	{
		ResourceID imported_resource_id;
		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();

		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
		if (file_system.load_file(Vadon::Core::FileSystem::Path{ .path = resource_path.path, .root = resource_path.root_directory }, resource_file_buffer) == false)
		{
			return imported_resource_id;
		}

		// FIXME: determine whether it's binary or text and use appropriate serializer!
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);
		if (serializer->initialize() == false)
		{
			log_error(std::format("Resource system error: failed to load resource file \"{}\"!\n", resource_path.path));
			return imported_resource_id;
		}

		// FIXME: should not have to load entire resource file just to register ID
		if (imported_resource_id.serialize(*serializer, "id") == false)
		{
			invalid_resource_data_error(resource_path);
			return ResourceID();
		}

		// Get type
		std::string resource_type;
		if (serializer->serialize("type", resource_type) == false)
		{
			invalid_resource_data_error(resource_path);
			return ResourceID();
		}

		const Vadon::Utilities::TypeID resource_type_id = Vadon::Utilities::TypeRegistry::get_type_id(resource_type);
		if (resource_type_id == Vadon::Utilities::c_invalid_type_id)
		{
			log_error(std::format("Resource system error: resource file \"{}\" uses unknown type \"{}\"!\n", resource_path.path, resource_type));
			return ResourceID();
		}

		// Add to lookup
		auto resource_info_it = m_resource_lookup.find(imported_resource_id);
		if (resource_info_it == m_resource_lookup.end())
		{
			ResourceInfo& new_resource_info = m_resource_lookup[imported_resource_id];
			new_resource_info.path = resource_path;
			new_resource_info.type_id = resource_type_id;
		}
		else
		{
			log_warning(std::format("Resource at path \"{}\" already imported!\n", resource_path.path));
		}

		return imported_resource_id;
	}

	bool ResourceSystem::export_resource(ResourceID resource_id)
	{
		ResourceInfo* resource_info = internal_find_resource(resource_id);
		if (resource_info == nullptr)
		{
			resource_not_found_error(resource_id);
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
		if (resource_id.serialize(*serializer, "id") == false)
		{
			resource_failed_to_serialize();
			return false;
		}

		{
			std::string resource_type = Vadon::Utilities::TypeRegistry::get_type_info(resource_info->type_id).name;
			if (serializer->serialize("type", resource_type) == false)
			{
				resource_failed_to_serialize();
				return false;
			}
		}

		Vadon::Scene::ResourceBase* resource = Vadon::Scene::ResourceSystem::get_resource_base(resource_info->handle);
		if (serialize_resource_data(*this, *serializer, *resource, *resource_info) == false)
		{
			return false;
		}

		if (serializer->finalize() == false)
		{
			resource_failed_to_serialize();
			return false;
		}

		if (m_engine_core.get_system<Vadon::Core::FileSystem>().save_file(Vadon::Core::FileSystem::Path{ .path = resource_info->path.path, .root = resource_info->path.root_directory }, resource_data_buffer) == false)
		{
			return false;
		}

		return true;
	}

	bool ResourceSystem::serialize_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value)
	{
		if (serializer.is_reading() == true)
		{
			Vadon::Utilities::Variant resource_id_variant;
			if (serializer.serialize(property_name, resource_id_variant) == true)
			{
				property_value = load_resource(std::get<ResourceID>(resource_id_variant));
			}
			else
			{
				// TODO: should use serializer codes to indicate whether an error occurred, or the property was just not present
				return false;
			}
		}
		else
		{
			Vadon::Utilities::Variant resource_id_variant = get_resource_id(std::get<ResourceHandle>(property_value));
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
			if (m_resource_lookup.find(new_resource_id) == m_resource_lookup.end())
			{
				break;
			}
		}

		// Add info to lookup
		ResourceInfo& new_resource_info = m_resource_lookup[new_resource_id];
		new_resource_info.type_id = type_id;

		return internal_create_resource(new_resource_id, new_resource_info, resource);
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

	const ResourceSystem::ResourceInfo* ResourceSystem::internal_find_resource(ResourceID resource_id) const
	{
		auto resource_handle_it = m_resource_lookup.find(resource_id);
		if (resource_handle_it == m_resource_lookup.end())
		{
			return nullptr;
		}

		return &resource_handle_it->second;
	}

	ResourceHandle ResourceSystem::internal_create_resource(ResourceID resource_id, ResourceInfo& info, ResourceBase* resource)
	{
		ResourceHandle new_resource_handle = m_resource_pool.add();
		ResourceData& new_resource_data = m_resource_pool.get(new_resource_handle);

		new_resource_data.id = resource_id;
		new_resource_data.resource = resource;

		info.handle = new_resource_handle;

		return new_resource_handle;
	}

	ResourceHandle ResourceSystem::load_resource(Vadon::Scene::ResourceSystemInterface& context, ResourceID resource_id)
	{
		// First check whether it's loaded already
		ResourceInfo* resource_info = internal_find_resource(resource_id);
		if (resource_info == nullptr)
		{
			resource_not_found_error(resource_id);
			return ResourceHandle();
		}

		if (resource_info->handle.is_valid() == true)
		{
			// Assume we already loaded this resource
			// FIXME: some way to detect whether this happened due to conflicting resource IDs?
			return resource_info->handle;
		}

		// Resource not yet loaded
		if (&context == this)
		{
			LoadContext load_context(*this, resource_id);
			return internal_load_resource(load_context, resource_id, *resource_info);
		}
		else
		{
			return internal_load_resource(context, resource_id, *resource_info);
		}
	}

	ResourceHandle ResourceSystem::internal_load_resource(Vadon::Scene::ResourceSystemInterface& context, ResourceID resource_id, ResourceInfo& info)
	{
		ResourceHandle loaded_resource_handle;

		if (info.path.is_valid() == false)
		{
			// TODO: print resource ID!
			log_error("Resource system error: unable to load resource with invalid path!\n");
			return loaded_resource_handle;
		}

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();

		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
		if (file_system.load_file(Vadon::Core::FileSystem::Path{ .path = info.path.path, .root = info.path.root_directory }, resource_file_buffer) == false)
		{
			return loaded_resource_handle;
		}

		// FIXME: determine whether it's binary or text and use appropriate serializer!
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);
		if (serializer->initialize() == false)
		{
			log_error(std::format("Resource system error: failed to load resource file \"{}\"!\n", info.path.path));
			return loaded_resource_handle;
		}

		// TODO: check to make sure ID and type matches?

		// Create new resource
		ResourceBase* loaded_resource = Vadon::Scene::ResourceRegistry::create_resource(info.type_id);

		if (serialize_resource_data(context, *serializer, *loaded_resource, info) == false)
		{
			delete loaded_resource;
			return loaded_resource_handle;
		}

		// Add to pool
		return internal_create_resource(resource_id, info, loaded_resource);
	}

	bool ResourceSystem::serialize_resource_data(Vadon::Scene::ResourceSystemInterface& context, Vadon::Utilities::Serializer& serializer, ResourceBase& resource, const ResourceInfo& info)
	{
		Vadon::Scene::ResourceRegistry::SerializerFunction serializer_function = Vadon::Scene::ResourceRegistry::get_serializer(info.type_id);
		if (serializer.is_reading() == true)
		{
			// Get list of properties to load (if using custom serializer, only load the base properties)
			const Vadon::Utilities::PropertyInfoList resource_properties = (serializer_function == nullptr) ?
				Vadon::Utilities::TypeRegistry::get_type_properties(info.type_id) : Vadon::Utilities::TypeRegistry::get_type_properties<ResourceBase>();

			for (const Vadon::Utilities::PropertyInfo& current_property_info : resource_properties)
			{
				Vadon::Utilities::Variant current_property_value;
				if (serializer.serialize(current_property_info.name, current_property_value) == true)
				{
					Vadon::Utilities::TypeRegistry::set_property(&resource, info.type_id, current_property_info.name, current_property_value);
				}
				else
				{
					invalid_resource_data_error(info.path);
					return false;
				}
			}
		}
		else
		{
			// Get list of properties to save (if using custom serializer, only save the base properties)
			Vadon::Utilities::PropertyList resource_properties = (serializer_function == nullptr) ?
				Vadon::Utilities::TypeRegistry::get_properties(&resource, info.type_id) : Vadon::Utilities::TypeRegistry::get_properties(&resource, Vadon::Utilities::TypeRegistry::get_type_id<ResourceBase>());

			for (Vadon::Utilities::Property& current_property : resource_properties)
			{
				if (serializer.serialize(current_property.name, current_property.value) == false)
				{
					invalid_resource_data_error(info.path);
					return false;
				}
			}
		}

		if (serializer_function != nullptr)
		{
			// Use the custom serializer
			if (serializer_function(context, serializer, resource) == false)
			{
				log_error(std::format("Resource system error: failed to load resource from file \"{}\"!\n", info.path.path));
				return false;
			}
		}

		return true;
	}
}