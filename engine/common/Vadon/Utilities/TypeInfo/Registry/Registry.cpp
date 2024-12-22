#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/Logger.hpp>

#include <Vadon/Utilities/Enum/EnumClass.hpp>

#include <format>

namespace Vadon::Utilities
{
	namespace
	{
		PropertyInfo make_property_info(std::string_view name, const MemberVariableBindBase& property)
		{
			return PropertyInfo{ .name = std::string(name), .data_type = property.data_type,
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
				property.setter_function(object, VariantArgumentList { value_ptr, value_ptr + 1 });
			}
		}

		void type_not_found_error(TypeID type_id)
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: type ID {} not present in registry!\n", Vadon::Utilities::to_integral(type_id)));
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

	TypeID TypeRegistry::get_type_id(std::string_view type_name)
	{
		const std::string type_name_str(type_name);
		TypeRegistry& instance = get_registry_instance();

		auto type_id_it = instance.m_id_lookup.find(type_name_str);
		if (type_id_it == instance.m_id_lookup.end())
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: {} not present in registry!\n", type_name));
			return Vadon::Utilities::TypeID::INVALID;
		}

		return type_id_it->second;
	}

	bool TypeRegistry::is_base_of(TypeID base_id, TypeID type_id)
	{
		if (base_id == type_id)
		{
			return true;
		}

		const TypeRegistry& instance = get_registry_instance();
		TypeID current_type_id = type_id;
		while (current_type_id != Vadon::Utilities::TypeID::INVALID)
		{
			auto current_data_it = instance.m_type_lookup.find(current_type_id);
			if (current_data_it == instance.m_type_lookup.end())
			{
				type_not_found_error(type_id);
				return false;
			}

			const TypeData& current_type_data = current_data_it->second;
			if (current_type_data.info.id == base_id)
			{
				return true;
			}

			current_type_id = current_type_data.info.base_id;
		}

		return false;
	}

	TypeInfo TypeRegistry::get_type_info(TypeID type_id)
	{
		TypeRegistry& instance = get_registry_instance();

		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			type_not_found_error(type_id);
			return TypeInfo();
		}

		return type_data_it->second.info;
	}

	std::vector<TypeID> TypeRegistry::get_subclass_list(TypeID type_id)
	{
		TypeRegistry& instance = get_registry_instance();

		std::vector<TypeID> subclass_list;
		subclass_list.push_back(type_id);

		for (const auto& type_pair : instance.m_type_lookup)
		{
			if (type_pair.first == type_id)
			{
				continue;
			}
			if (is_base_of(type_id, type_pair.first) == true)
			{
				subclass_list.push_back(type_pair.first);
			}
		}

		return subclass_list;
	}

	PropertyInfoList TypeRegistry::get_type_properties(TypeID type_id)
	{
		PropertyInfoList properties;

		TypeRegistry& instance = get_registry_instance();
		instance.internal_get_type_properties(type_id, properties);

		return properties;
	}

	PropertyList TypeRegistry::get_properties(void* object, TypeID type_id)
	{
		PropertyList properties;

		TypeRegistry& instance = get_registry_instance();
		instance.internal_get_properties(object, type_id, properties);

		return properties;
	}

	Variant TypeRegistry::get_property(void* object, TypeID type_id, std::string_view property_name)
	{
		TypeRegistry& instance = get_registry_instance();
		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			type_not_found_error(type_id);
			return Variant();
		}

		const TypeData& type_data = type_data_it->second;
		const MemberVariableBindBase* property_bind = instance.internal_find_property(type_data, property_name);
		if (property_bind == nullptr)
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: property \"{}\" not found in type \"{}\"!\n", property_name, type_data.info.name));
			return Variant();
		}

		return invoke_property_getter(object, *property_bind);
	}

	void TypeRegistry::set_property(void* object, TypeID type_id, std::string_view property_name, const Variant& value)
	{
		TypeRegistry& instance = get_registry_instance();
		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			type_not_found_error(type_id);
			return;
		}

		const TypeData& type_data = type_data_it->second;
		const MemberVariableBindBase* property_bind = instance.internal_find_property(type_data, property_name);
		if (property_bind == nullptr)
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: property \"{}\" not found in type \"{}\"!\n", property_name, type_data.info.name));
			return;
		}

		invoke_property_setter(object, *property_bind, value);
	}

	void TypeRegistry::internal_register_type(std::string_view type_name, size_t size, size_t alignment, TypeID base_type_id)
	{
		const std::string type_name_str(type_name);
		TypeRegistry& instance = get_registry_instance();
		if (instance.m_id_lookup.find(type_name_str) == instance.m_id_lookup.end())
		{
			const TypeID new_type_id = to_enum<TypeID>(instance.m_id_counter++);
			instance.m_id_lookup.emplace(type_name_str, new_type_id);

			TypeData& new_type_data = instance.m_type_lookup.insert(std::make_pair(new_type_id, TypeData{})).first->second;
			new_type_data.info.id = new_type_id;
			new_type_data.info.name = type_name;
			new_type_data.info.size = size;
			new_type_data.info.alignment = alignment;

			if (base_type_id != Vadon::Utilities::TypeID::INVALID)
			{
				instance.register_type_with_base(new_type_id, new_type_data, base_type_id);
			}
		}
		else
		{
			// FIXME: more elegant error handling?
			Vadon::Core::Logger::log_error(std::format("Type registry error: \"{}\" already exists in registry!\n", type_name));
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
			type_not_found_error(type_id);
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
			type_not_found_error(type_id);
			return false;
		}

		return type_data_it->second.bind_method(name, std::move(method_bind));
	}

	void TypeRegistry::register_type_with_base(TypeID /*type_id*/, TypeRegistry::TypeData& data, TypeID base_id)
	{
		auto base_data_it = m_type_lookup.find(base_id);
		if (base_data_it == m_type_lookup.end())
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: base class with type ID {} provided for \"{}\" is not present in registry!\n", Vadon::Utilities::to_integral(base_id), data.info.name));
			return;
		}

		data.info.base_id = base_id;

		// TODO: build up LUTs to improve lookups for properties, etc.?
	}

	bool TypeRegistry::has_method(TypeID type_id, std::string_view method_name) const
	{
		// FIXME: have a faster way to look this up?
		TypeID current_type_id = type_id;
		while (current_type_id != Vadon::Utilities::TypeID::INVALID)
		{
			auto current_data_it = m_type_lookup.find(current_type_id);
			if (current_data_it == m_type_lookup.end())
			{
				type_not_found_error(type_id);
				return false;
			}

			const TypeData& current_type_data = current_data_it->second;
			if (current_type_data.has_method(method_name) == true)
			{
				return true;
			}

			current_type_id = current_type_data.info.base_id;
		}

		return false;
	}

	bool TypeRegistry::has_property(TypeID type_id, std::string_view property_name) const
	{
		// FIXME: have a faster way to look this up?
		TypeID current_type_id = type_id;
		while (current_type_id != Vadon::Utilities::TypeID::INVALID)
		{
			auto current_data_it = m_type_lookup.find(current_type_id);
			if (current_data_it == m_type_lookup.end())
			{
				type_not_found_error(type_id);
				return false;
			}

			const TypeData& current_type_data = current_data_it->second;
			if (current_type_data.has_property(property_name) == true)
			{
				return true;
			}

			current_type_id = current_type_data.info.base_id;
		}

		return false;
	}

	void TypeRegistry::internal_get_type_properties(TypeID type_id, PropertyInfoList& property_list) const
	{
		auto type_data_it = m_type_lookup.find(type_id);
		if (type_data_it == m_type_lookup.end())
		{
			type_not_found_error(type_id);
			return;
		}

		const TypeData& type_data = type_data_it->second;
		if (type_data.info.base_id != Vadon::Utilities::TypeID::INVALID)
		{
			internal_get_type_properties(type_data.info.base_id, property_list);
		}

		for (const auto& property_data : type_data.properties)
		{
			property_list.push_back(make_property_info(property_data.first, property_data.second));
		}
	}

	void TypeRegistry::internal_get_properties(void* object, TypeID type_id, PropertyList& property_list) const
	{
		auto type_data_it = m_type_lookup.find(type_id);
		if (type_data_it == m_type_lookup.end())
		{
			type_not_found_error(type_id);
			return;
		}

		const TypeData& type_data = type_data_it->second;
		if (type_data.info.base_id != Vadon::Utilities::TypeID::INVALID)
		{
			internal_get_properties(object, type_data.info.base_id, property_list);
		}

		for (const auto& current_property : type_data.properties)
		{
			const MemberVariableBindBase& property_bind = current_property.second;
			if (property_bind.has_getter() == false)
			{
				continue;
			}

			property_list.emplace_back(current_property.first, current_property.second.data_type, invoke_property_getter(object, property_bind));
		}
	}

	const MemberVariableBindBase* TypeRegistry::internal_find_property(const TypeData& type_data, std::string_view name) const
	{
		auto property_it = type_data.properties.find(std::string(name));
		if (property_it != type_data.properties.end())
		{
			return &property_it->second;
		}

		if (type_data.info.base_id != Vadon::Utilities::TypeID::INVALID)
		{
			auto base_data_it = m_type_lookup.find(type_data.info.base_id);
			if (base_data_it == m_type_lookup.end())
			{
				type_not_found_error(type_data.info.base_id);
				return nullptr;
			}

			const MemberVariableBindBase* base_property = internal_find_property(base_data_it->second, name);
			if (base_property != nullptr)
			{
				return base_property;
			}
		}

		return nullptr;
	}
}