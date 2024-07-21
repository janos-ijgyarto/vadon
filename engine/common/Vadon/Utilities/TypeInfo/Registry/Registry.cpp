#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/Logger.hpp>

#include <format>

namespace Vadon::Utilities
{
	namespace
	{
		PropertyInfo make_property_info(std::string_view name, const MemberVariableBindBase& property)
		{			
			return PropertyInfo{ .name = std::string(name), .type_index = property.type,
				.has_getter = property.member_getter || property.getter_function,
				.has_setter = property.member_setter || property.setter_function };
		}

		TypeRegistry& get_registry_instance()
		{
			return Vadon::Core::EngineEnvironment::get_type_registry();
		}

		Variant invoke_property_getter(void* object, const MemberVariableBindBase& property)
		{
			if (property.member_getter != nullptr)
			{
				return property.member_getter(object);
			}
			else
			{
				return property.getter_function(object, VariantArgumentList());
			}
		}

		void invoke_property_setter(void* object, const MemberVariableBindBase& property, const Variant& value)
		{
			if (property.member_setter != nullptr)
			{
				property.member_setter(object, value);
			}
			else
			{
				Variant* value_ptr = const_cast<Variant*>(&value);
				property.setter_function(object, VariantArgumentList{ value_ptr, value_ptr + 1 });
			}
		}
	}

	bool TypeRegistry::TypeData::bind_method(std::string_view method_name, MemberFunctionBind method)
	{
		if (has_method(method_name) == true)
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: \"{}\" already has method registered with name \"{}\"!\n", info.name, method_name));
			return false;
		}

		methods.emplace(std::string(method_name), method);
		return true;
	}

	bool TypeRegistry::TypeData::add_property(std::string_view name, MemberVariableBindBase property)
	{
		if (has_property(name) == true)
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: \"{}\" already has property registered with name \"{}\"!\n", info.name, name));
			return false;
		}

		properties.emplace(std::string(name), property);
		return true;
	}

	bool TypeRegistry::TypeData::has_method(std::string_view method_name) const
	{
		auto method_it = methods.find(std::string(method_name));
		if (method_it != methods.end())
		{
			return true;
		}

		return false;
	}

	bool TypeRegistry::TypeData::has_property(std::string_view property_name) const
	{
		auto property_it = properties.find(std::string(property_name));
		if (property_it != properties.end())
		{
			return true;
		}

		return false;
	}

	uint32_t TypeRegistry::get_type_id(std::string_view type_name)
	{
		const std::string type_name_str(type_name);
		TypeRegistry& instance = get_registry_instance();

		auto type_id_it = instance.m_id_lookup.find(type_name_str);
		if (type_id_it == instance.m_id_lookup.end())
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: {} not present in registry!\n", type_name));
			return c_invalid_type_id;
		}

		return type_id_it->second;
	}

	TypeInfo TypeRegistry::get_type_info(uint32_t type_id)
	{
		TypeRegistry& instance = get_registry_instance();

		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			// TODO: error?
			return TypeInfo();
		}

		return type_data_it->second.info;
	}

	PropertyInfoList TypeRegistry::get_type_properties(uint32_t type_id)
	{
		TypeRegistry& instance = get_registry_instance();

		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			// TODO: error?
			return PropertyInfoList();
		}

		PropertyInfoList properties;
		const TypeData& type_data = type_data_it->second;
		for (const auto& property_data : type_data.properties)
		{
			properties.push_back(make_property_info(property_data.first, property_data.second));
		}

		return properties;
	}

	PropertyList TypeRegistry::get_properties(void* object, uint32_t type_id)
	{
		PropertyList properties;
		TypeRegistry& instance = get_registry_instance();

		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			// TODO: error?
			return properties;
		}

		const TypeData& type_data = type_data_it->second;
		// FIXME: support inheritance?
		for (const auto& current_property : type_data.properties)
		{
			const MemberVariableBindBase& property_bind = current_property.second;
			if (property_bind.has_getter() == false)
			{
				continue;
			}

			properties.emplace_back(current_property.first, invoke_property_getter(object, property_bind));
		}

		return properties;
	}

	Variant TypeRegistry::get_property(void* object, uint32_t type_id, std::string_view property_name)
	{
		TypeRegistry& instance = get_registry_instance();
		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			// TODO: error?
			return Variant();
		}

		const TypeData& type_data = type_data_it->second;
		auto type_property_it = type_data.properties.find(std::string(property_name));
		if (type_property_it == type_data.properties.end())
		{
			// TODO: error?
			return Variant();
		}

		const MemberVariableBindBase& property_bind = type_property_it->second;
		return invoke_property_getter(object, property_bind);
	}

	void TypeRegistry::set_property(void* object, uint32_t type_id, std::string_view property_name, const Variant& value)
	{
		TypeRegistry& instance = get_registry_instance();
		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			// TODO: error?
			return;
		}

		const TypeData& type_data = type_data_it->second;
		auto type_property_it = type_data.properties.find(std::string(property_name));
		if (type_property_it == type_data.properties.end())
		{
			// TODO: error?
			return;
		}

		const MemberVariableBindBase& property_bind = type_property_it->second;
		invoke_property_setter(object, property_bind, value);
	}

	void TypeRegistry::internal_register_type(std::string_view type_name)
	{
		const std::string type_name_str(type_name);
		TypeRegistry& instance = get_registry_instance();
		if (instance.m_id_lookup.find(type_name_str) == instance.m_id_lookup.end())
		{
			const uint32_t new_type_id = instance.m_id_counter++;
			instance.m_id_lookup.emplace(type_name_str, new_type_id);

			TypeData& new_type_data = instance.m_type_lookup.insert(std::make_pair(new_type_id, TypeData{})).first->second;
			new_type_data.info.id = new_type_id;
			new_type_data.info.name = type_name;
		}
		else
		{
			// FIXME: more elegant error handling?
			Vadon::Core::Logger::log_error(std::format("Type registry error: {} already exists in registry!\n", type_name));
			assert(false);
			std::terminate();
		}
	}

	bool TypeRegistry::internal_add_property(TypeID type_id, std::string_view name, MemberVariableBindBase property_bind)
	{
		TypeRegistry& instance = get_registry_instance(); 
		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			// TODO: error?
			return false;
		}

		return type_data_it->second.add_property(name, std::move(property_bind));
	}

	bool TypeRegistry::internal_bind_method(TypeID type_id, std::string_view name, MemberFunctionBind method_bind)
	{
		TypeRegistry& instance = get_registry_instance();
		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			// TODO: error?
			return false;
		}

		return type_data_it->second.bind_method(name, std::move(method_bind));
	}
}