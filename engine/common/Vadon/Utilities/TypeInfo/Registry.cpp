#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/TypeInfo/Registry.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/Logger.hpp>

#include <Vadon/Utilities/Enum/EnumClass.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>

#include <format>

namespace Vadon::Utilities
{
	namespace
	{
		PropertyInfo make_property_info(const ::Vadon::Foundation::Property& property_info, const MemberVariableBindBase& property)
		{
			return PropertyInfo{ .base_info = property_info,
				.has_getter = property.member_getter || property.getter_function,
				.has_setter = property.member_setter || property.setter_function
			};
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

	TypeRegistry::TypeRegistry()
	{
	}

	bool TypeRegistry::initialize()
	{
		if (m_initialized == true)
		{
			return true;
		}

		m_initialized = true;

		// Register all base types first to ensure that their TypeIDs are added first
		register_type<int>();
		register_type<uint32_t>();
		register_type<float>();
		register_type<bool>();
		register_type<std::string>();
		register_type<Vadon::Math::Vector2>();
		register_type<Vadon::Math::Vector2i>();
		register_type<Vadon::Math::Vector3>();
		register_type<Vadon::Math::Vector3i>();
		register_type<Vadon::Math::Vector4>();
		register_type<Vadon::Math::ColorRGBA>();
		register_type<::Vadon::Foundation::UUID>();

		register_type<BoxedVariantArray>();

		return true;
	}

	bool TypeRegistry::TypeData::bind_method(const MemberFunctionUUID& method_uuid, MemberFunctionBind method)
	{
		if (has_method(method_uuid) == true)
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: \"{}\" already has method registered with name \"{}\"!\n", uuid_to_string(info.id), uuid_to_string(method_uuid)));
			return false;
		}

		methods.emplace(method_uuid, method);
		return true;
	}

	bool TypeRegistry::TypeData::add_property(const PropertyUUID& property_uuid, const ::Vadon::Foundation::Property& property_info, MemberVariableBindBase member_bind)
	{
		if (has_property(property_uuid) == true)
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: \"{}\" already has property registered with name \"{}\"!\n", uuid_to_string(info.id), uuid_to_string(property_uuid)));
			return false;
		}

		properties.emplace(property_uuid, PropertyData{ .info = property_info, .member_bind = member_bind });
		return true;
	}

	bool TypeRegistry::TypeData::has_method(const MemberFunctionUUID& method_uuid) const
	{
		auto method_it = methods.find(method_uuid);
		if (method_it != methods.end())
		{
			return true;
		}

		return false;
	}

	bool TypeRegistry::TypeData::has_property(const PropertyUUID& property_uuid) const
	{
		auto property_it = properties.find(property_uuid);
		if (property_it != properties.end())
		{
			return true;
		}

		return false;
	}

	TypeID TypeRegistry::get_type_id(const TypeUUID& type_uuid)
	{
		TypeRegistry& instance = get_registry_instance();

		auto type_id_it = instance.m_id_lookup.find(type_uuid);
		if (type_id_it == instance.m_id_lookup.end())
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: {} not present in registry!\n", uuid_to_string(type_uuid)));
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
			if (current_type_id == base_id)
			{
				return true;
			}

			auto current_data_it = instance.m_type_lookup.find(current_type_id);
			if (current_data_it == instance.m_type_lookup.end())
			{
				type_not_found_error(type_id);
				return false;
			}

			const TypeData& current_type_data = current_data_it->second;
			current_type_id = current_type_data.base_id;
		}

		return false;
	}

	::Vadon::Foundation::TypeInfo TypeRegistry::get_type_info(TypeID type_id)
	{
		TypeRegistry& instance = get_registry_instance();

		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			type_not_found_error(type_id);
			return ::Vadon::Foundation::TypeInfo();
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

	PropertyInfo TypeRegistry::get_property_info(TypeID type_id, const PropertyUUID& property_uuid)
	{
		TypeRegistry& instance = get_registry_instance();
		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			type_not_found_error(type_id);
			return PropertyInfo();
		}

		const PropertyData* property_data = instance.internal_find_property(type_data_it->second, property_uuid);
		if (property_data == nullptr)
		{
			// TODO: error!
			return PropertyInfo();
		}

		return make_property_info(property_data->info, property_data->member_bind);
	}

	Variant TypeRegistry::get_property(void* object, TypeID type_id, const PropertyUUID& property_uuid)
	{
		TypeRegistry& instance = get_registry_instance();
		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			type_not_found_error(type_id);
			return Variant();
		}

		const TypeData& type_data = type_data_it->second;
		const PropertyData* property_data = instance.internal_find_property(type_data, property_uuid);
		if (property_data == nullptr)
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: property \"{}\" not found in type \"{}\"!\n", uuid_to_string(property_uuid), uuid_to_string(type_data.info.id)));
			return Variant();
		}

		return invoke_property_getter(object, property_data->member_bind);
	}

	void TypeRegistry::set_property(void* object, TypeID type_id, const PropertyUUID& property_uuid, const Variant& value)
	{
		TypeRegistry& instance = get_registry_instance();
		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			type_not_found_error(type_id);
			return;
		}

		const TypeData& type_data = type_data_it->second;
		instance.internal_apply_property_value(type_data, object, property_uuid, value);
	}

	void TypeRegistry::apply_property_values(void* object, TypeID type_id, const PropertyList& properties)
	{
		TypeRegistry& instance = get_registry_instance();
		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			type_not_found_error(type_id);
			return;
		}

		const TypeData& type_data = type_data_it->second;
		for (const Property& current_property : properties)
		{
			instance.internal_apply_property_value(type_data, object, current_property.info.id, current_property.value);
		}
	}

	std::vector<TypeUUID> TypeRegistry::get_all_registered_types()
	{
		std::vector<TypeUUID> type_list;

		TypeRegistry& instance = get_registry_instance();
		for (const auto& type_id_pair : instance.m_id_lookup)
		{
			type_list.push_back(type_id_pair.first);
		}

		return type_list;
	}

	void TypeRegistry::internal_register_type(const TypeUUID& type_uuid, size_t size, size_t alignment, TypeID base_type_id)
	{
		TypeRegistry& instance = get_registry_instance();

		VADON_ASSERT(instance.m_id_lookup.find(type_uuid) == instance.m_id_lookup.end(), std::format("Type registry error: \"{}\" already exists in registry!\n", uuid_to_string(type_uuid)));

		const TypeID new_type_id = to_enum<TypeID>(instance.m_id_counter++);
		instance.m_id_lookup.emplace(type_uuid, new_type_id);

		TypeData& new_type_data = instance.m_type_lookup.insert(std::make_pair(new_type_id, TypeData{})).first->second;
		new_type_data.info.id = type_uuid;
		new_type_data.info.size = size;
		new_type_data.info.alignment = alignment;
		new_type_data.info.property_count = 0;

		if (base_type_id != Vadon::Utilities::TypeID::INVALID)
		{
			instance.register_type_with_base(new_type_id, new_type_data, base_type_id);
		}
	}

	bool TypeRegistry::internal_add_property(TypeID type_id, const PropertyUUID& property_uuid, MemberVariableBindBase property_bind)
	{
		TypeRegistry& instance = get_registry_instance();

		auto type_data_it = instance.m_type_lookup.find(type_id);
		VADON_ASSERT(type_data_it != instance.m_type_lookup.end(), "Type not found!");
		if (type_data_it == instance.m_type_lookup.end())
		{
			type_not_found_error(type_id);
			return false;
		}

		// Make sure the property is itself a registered type
		auto property_type_it = instance.m_type_lookup.find(property_bind.type);
		if (type_data_it == instance.m_type_lookup.end())
		{
			type_not_found_error(property_bind.type);
			return false;
		}

		const TypeData& property_type_data = property_type_it->second;
		::Vadon::Foundation::Property property_info = { .id = property_uuid, .type = property_type_data.info.id };

		return type_data_it->second.add_property(property_uuid, property_info, std::move(property_bind));
	}

	bool TypeRegistry::internal_bind_method(TypeID type_id, const MemberFunctionUUID& method_uuid, MemberFunctionBind method_bind)
	{
		TypeRegistry& instance = get_registry_instance();
		auto type_data_it = instance.m_type_lookup.find(type_id);
		if (type_data_it == instance.m_type_lookup.end())
		{
			type_not_found_error(type_id);
			return false;
		}

		return type_data_it->second.bind_method(method_uuid, std::move(method_bind));
	}

	void TypeRegistry::register_type_with_base(TypeID /*type_id*/, TypeRegistry::TypeData& data, TypeID base_id)
	{
		auto base_data_it = m_type_lookup.find(base_id);
		if (base_data_it == m_type_lookup.end())
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: base class with type ID {} provided for \"{}\" is not present in registry!\n", Vadon::Utilities::to_integral(base_id), uuid_to_string(data.info.id)));
			return;
		}

		data.info.base_id = base_data_it->second.info.id;
		data.base_id = base_id;

		// TODO: build up LUTs to improve lookups for properties, etc.?
	}

	bool TypeRegistry::has_method(TypeID type_id, const MemberFunctionUUID& method_uuid) const
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
			if (current_type_data.has_method(method_uuid) == true)
			{
				return true;
			}

			current_type_id = current_type_data.base_id;
		}

		return false;
	}

	bool TypeRegistry::has_property(TypeID type_id, const PropertyUUID& property_uuid) const
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
			if (current_type_data.has_property(property_uuid) == true)
			{
				return true;
			}

			current_type_id = current_type_data.base_id;
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
		if (type_data.base_id != Vadon::Utilities::TypeID::INVALID)
		{
			internal_get_type_properties(type_data.base_id, property_list);
		}

		for (const auto& property_data_pair : type_data.properties)
		{
			const PropertyData& current_property_data = property_data_pair.second;
			property_list.push_back(make_property_info(current_property_data.info, current_property_data.member_bind));
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
		if (type_data.base_id != Vadon::Utilities::TypeID::INVALID)
		{
			internal_get_properties(object, type_data.base_id, property_list);
		}

		for (const auto& current_property : type_data.properties)
		{
			const PropertyData& current_property_data = current_property.second;
			if (current_property_data.member_bind.has_getter() == false)
			{
				continue;
			}

			property_list.emplace_back(current_property_data.info, invoke_property_getter(object, current_property_data.member_bind));
		}
	}

	const TypeRegistry::PropertyData* TypeRegistry::internal_find_property(const TypeData& type_data, const PropertyUUID& property_uuid) const
	{
		auto property_it = type_data.properties.find(property_uuid);
		if (property_it != type_data.properties.end())
		{
			return &property_it->second;
		}

		if (type_data.base_id != Vadon::Utilities::TypeID::INVALID)
		{
			auto base_data_it = m_type_lookup.find(type_data.base_id);
			if (base_data_it == m_type_lookup.end())
			{
				type_not_found_error(type_data.base_id);
				return nullptr;
			}

			const PropertyData* base_property = internal_find_property(base_data_it->second, property_uuid);
			if (base_property != nullptr)
			{
				return base_property;
			}
		}

		return nullptr;
	}

	void TypeRegistry::internal_apply_property_value(const TypeData& type_data, void* object, const PropertyUUID& property_uuid, const Variant& value)
	{
		const PropertyData* property = internal_find_property(type_data, property_uuid);
		if (property == nullptr)
		{
			Vadon::Core::Logger::log_error(std::format("Type registry error: property \"{}\" not found in type \"{}\"!\n", Vadon::Utilities::uuid_to_string(property_uuid), Vadon::Utilities::uuid_to_string(type_data.info.id)));
			return;
		}

		invoke_property_setter(object, property->member_bind, value);
	}
}