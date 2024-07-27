#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <format>

namespace
{
	void invalid_resource_data_error(std::string_view path)
	{
		Vadon::Core::Logger::log_error(std::format("Resource system error: invalid resource data in file \"{}\"!\n", path));
	}
}

namespace Vadon::Private::Scene
{
	const ResourceBase* ResourceSystem::get_resource_base(ResourceID resource_id) const
	{
		ResourceHandle resource_handle = internal_find_resource(resource_id);
		if (resource_handle.is_valid() == false)
		{
			return nullptr;
		}

		return m_resource_pool.get(resource_handle).resource;
	}

	ResourceID ResourceSystem::get_resource_id(ResourceHandle resource_handle) const
	{
		const ResourceData& resource_data = m_resource_pool.get(resource_handle);
		return resource_data.id;
	}

	ResourceHandle ResourceSystem::load_resource(ResourceID resource_id)
	{
		// TODO: support loading from file if it hasn't been loaded yet
		return internal_find_resource(resource_id);
	}

	ResourceHandle ResourceSystem::load_resource(std::string_view resource_path)
	{
		ResourceHandle loaded_resource_handle;

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();

		Vadon::Core::FileSystem::RawFileDataBuffer resource_file_buffer;
		if (file_system.load_file(resource_path, resource_file_buffer) == false)
		{
			return loaded_resource_handle;
		}

		// FIXME: determine whether it's binary or text and use appropriate serializer!
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(resource_file_buffer, Vadon::Utilities::Serializer::Type::JSON, true);
		if (serializer->initialize() == false)
		{
			log_error(std::format("Resource system error: failed to load resource file \"{}\"!\n", resource_path));
			return loaded_resource_handle;
		}

		ResourceID loaded_resource_id;
		if (loaded_resource_id.serialize(*serializer, "id") == false)
		{
			invalid_resource_data_error(resource_path);
			return loaded_resource_handle;
		}

		loaded_resource_handle = internal_find_resource(loaded_resource_id);
		if (loaded_resource_handle.is_valid() == true)
		{
			// Assume we already loaded this resource
			// FIXME: some way to detect whether this happened due to conflicting resource IDs?
			return loaded_resource_handle;
		}

		// Get type
		std::string resource_type;
		if (serializer->serialize("type", resource_type) == false)
		{
			invalid_resource_data_error(resource_path);
			return loaded_resource_handle;
		}

		const Vadon::Utilities::TypeID resource_type_id = Vadon::Utilities::TypeRegistry::get_type_id(resource_type);
		if (resource_type_id == Vadon::Utilities::c_invalid_type_id)
		{
			log_error(std::format("Resource system error: resource file \"{}\" uses unknown type \"{}\"!\n", resource_path, resource_type));
			return loaded_resource_handle;
		}

		// Create new resource
		ResourceBase* loaded_resource = Vadon::Scene::ResourceRegistry::create_resource(resource_type_id);

		// Load property values from file
		const Vadon::Utilities::PropertyInfoList resource_properties = Vadon::Utilities::TypeRegistry::get_type_properties(resource_type_id);

		for (const Vadon::Utilities::PropertyInfo& current_property_info : resource_properties)
		{
			Vadon::Utilities::Variant current_property_value;
			if (serializer->serialize(current_property_info.name, current_property_value) == true)
			{
				Vadon::Utilities::TypeRegistry::set_property(loaded_resource, resource_type_id, current_property_info.name, current_property_value);
			}
		}		

		// Add to pool
		return internal_create_resource(loaded_resource_id, loaded_resource);
	}

	ResourceHandle ResourceSystem::internal_create_resource(ResourceBase* resource)
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

		return internal_create_resource(new_resource_id, resource);
	}

	ResourceSystem::ResourceSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Scene::ResourceSystem(core)
	{}

	bool ResourceSystem::initialize()
	{
		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();
		std::vector<std::string> resource_files = file_system.get_files_of_type("", ".vdrs", true);

		for (const std::string& resource_path : resource_files)
		{
			load_resource(resource_path);
		}

		return true;
	}

	ResourceHandle ResourceSystem::internal_find_resource(ResourceID resource_id) const
	{
		auto resource_handle_it = m_resource_lookup.find(resource_id);
		if (resource_handle_it == m_resource_lookup.end())
		{
			return ResourceHandle();
		}

		return resource_handle_it->second;
	}

	ResourceHandle ResourceSystem::internal_create_resource(ResourceID resource_id, ResourceBase* resource)
	{
		ResourceHandle new_resource_handle = m_resource_pool.add();
		ResourceData& new_resource_data = m_resource_pool.get(new_resource_handle);

		new_resource_data.id = resource_id;
		new_resource_data.resource = resource;

		// Add to lookup
		m_resource_lookup[resource_id] = new_resource_handle;

		return new_resource_handle;
	}
}