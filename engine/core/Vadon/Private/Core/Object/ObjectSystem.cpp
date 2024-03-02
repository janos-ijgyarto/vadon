#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Core/Object/ObjectSystem.hpp>

#include <format>
#include <deque>

namespace Vadon::Private::Core
{
	namespace
	{
		std::string get_pretty_name(std::string_view pretty_name, std::string_view class_id)
		{
			if (pretty_name.empty() == false)
			{
				return std::string(pretty_name);
			}

			constexpr std::string_view c_namespace_operator = "::";
			const size_t trim_offset = class_id.find_last_of(c_namespace_operator);
			if (trim_offset != std::string::npos)
			{
				return std::string(class_id.substr(trim_offset + (c_namespace_operator.size() - 1)));
			}

			return std::string(class_id);
		}
	}

	bool ObjectSystem::ObjectClassData::internal_bind_method(std::string_view name, MemberFunctionData method)
	{
		if(has_method(name) == true)
		{
			engine_core.get_logger().error(std::format("Class \"{}\" already has method registered with name \"{}\"!\n", class_info.name, name));
			return false;
		}

		methods.emplace(std::string(name), method);
		return true;
	}

	bool ObjectSystem::ObjectClassData::add_property(ObjectPropertyInfo property_info)
	{
		if (has_property(property_info.name) == true)
		{
			engine_core.get_logger().error(std::format("Class \"{}\" already has property registered with name \"{}\"!\n", class_info.name, property_info.name));
			return false;
		}

		// FIXME: find some way to deduplicate the steps below?
		// Validate getter
		{
			auto getter_it = methods.find(std::string(property_info.getter));
			if (getter_it == methods.end())
			{
				engine_core.get_logger().error(std::format("Class \"{}\" has no getter for property \"{}\" named \"{}\"!\n", class_info.name, property_info.name, property_info.getter));
				return false;
			}

			// TODO: further restrictions to make sure getter is of the correct format (i.e no args)
		}

		// Validate setter
		{
			auto setter_it = methods.find(std::string(property_info.setter));
			if (setter_it == methods.end())
			{
				engine_core.get_logger().error(std::format("Class has no setter for property \"{}\" named \"{}\"!\n", class_info.name, property_info.setter));
				return false;
			}

			// TODO: further restrictions to make sure setter is of the correct format (i.e one argument, matches property type)
		}

		properties.emplace(std::string(property_info.name), property_info);
		return true;
	}

	bool ObjectSystem::ObjectClassData::has_method(std::string_view name) const
	{
		auto method_it = methods.find(std::string(name));
		if (method_it != methods.end())
		{
			return true;
		}

		return false;
	}

	bool ObjectSystem::ObjectClassData::has_property(std::string_view name) const
	{
		auto property_it = properties.find(std::string(name));
		if (property_it != properties.end())
		{
			return true;
		}

		return false;
	}

	Object* ObjectSystem::create_object(std::string_view class_id)
	{
		auto class_data_it = m_object_classes.find(std::string(class_id));
		if (class_data_it == m_object_classes.end())
		{
			// TODO: error?
			return nullptr;
		}

		const ObjectClassData& class_data = class_data_it->second;
		return class_data.factory(m_engine_core);
	}

	ObjectClassInfo ObjectSystem::get_class_info(std::string_view class_id) const
	{
		auto class_data_it = m_object_classes.find(std::string(class_id));
		if (class_data_it != m_object_classes.end())
		{
			const ObjectClassData& class_data = class_data_it->second;
			return class_data.class_info;
		}

		return ObjectClassInfo();
	}

	ObjectClassInfoList ObjectSystem::get_class_list() const
	{
		ObjectClassInfoList class_info_list;
		for (const auto& current_entry : m_object_classes)
		{
			class_info_list.push_back(current_entry.second.class_info);
		}

		return class_info_list;
	}

	ObjectClassInfoList ObjectSystem::get_subclass_hierarchy(std::string_view class_id) const
	{
		ObjectClassInfoList subclass_list;

		auto class_data_it = m_object_classes.find(std::string(class_id));
		if (class_data_it == m_object_classes.end())
		{
			// TODO: error!
			return subclass_list;
		}

		const ObjectClassData& root_class_data = class_data_it->second;
		subclass_list.push_back(root_class_data.class_info);

		// Use deque to safely append and pop from front until empty
		std::deque<ObjectClassData*> subclass_queue(root_class_data.subclasses.begin(), root_class_data.subclasses.end());
		while (subclass_queue.empty() == false)
		{
			// Add subclass info
			ObjectClassData* current_subclass = subclass_queue.front();
			subclass_list.push_back(current_subclass->class_info);

			// Append subclasses
			subclass_queue.insert(subclass_queue.end(), current_subclass->subclasses.begin(), current_subclass->subclasses.end());

			// Remove the most recent entry
			subclass_queue.pop_front();
		}

		return subclass_list;
	}

	ObjectPropertyInfoList ObjectSystem::get_class_properties(std::string_view class_id, bool recursive) const
	{
		ObjectPropertyInfoList property_list;

		auto class_data_it = m_object_classes.find(std::string(class_id));
		if (class_data_it == m_object_classes.end())
		{
			// TODO: error?
			return property_list;
		}

		// Start with own properties
		const ObjectClassData* current_class_data = &class_data_it->second;
		while (current_class_data != nullptr)
		{
			property_list.reserve(property_list.size() + current_class_data->properties.size());
			for (const auto& current_property : current_class_data->properties)
			{
				property_list.push_back(current_property.second);
			}
			if (recursive == true)
			{
				// Get properties of the rest of the inheritance chain
				current_class_data = current_class_data->base_data;
			}
			else
			{
				current_class_data = nullptr;
			}
		}

		return property_list;
	}

	ObjectPropertyList ObjectSystem::get_object_properties(Object& object, bool recursive) const
	{
		ObjectPropertyList properties;

		auto class_data_it = m_object_classes.find(std::string(object.get_class_id()));
		if (class_data_it == m_object_classes.end())
		{
			// TODO: error?
			return properties;
		}

		const ObjectClassData* current_class_data = &class_data_it->second;
		while (current_class_data != nullptr)
		{
			for (auto& current_property : current_class_data->properties)
			{
				const ObjectPropertyInfo& property_info = current_property.second;

				// FIXME: properties should store an index so we don't have to do this lookup!
				const MemberFunctionData& property_getter = current_class_data->methods.find(property_info.getter)->second;

				properties.emplace_back(property_info.name, invoke_method(object, property_info.getter, property_getter));
			}

			if (recursive == true)
			{
				current_class_data = current_class_data->base_data;
			}
			else
			{
				current_class_data = nullptr;
			}
		}

		return properties;
	}

	Variant ObjectSystem::get_property(Object& object, std::string_view property_name) const
	{
		auto class_data_it = m_object_classes.find(std::string(object.get_class_id()));
		if (class_data_it == m_object_classes.end())
		{
			// TODO: error?
			return Variant();
		}

		const ObjectClassData& class_data = class_data_it->second;
		const ObjectPropertyLookup property_lookup = internal_find_property_info(class_data, property_name);
		if(property_lookup.is_valid() == false)
		{
			// TODO: error?
			return Variant();
		}

		// FIXME: properties should store an index so we don't have to do this lookup!
		const MemberFunctionData& property_getter = property_lookup.class_data->methods.find(property_lookup.property_info->getter)->second;

		return invoke_method(object, property_lookup.property_info->getter, property_getter);
	}

	void ObjectSystem::set_property(Object& object, std::string_view property_name, Variant value)
	{
		auto class_data_it = m_object_classes.find(std::string(object.get_class_id()));
		if (class_data_it == m_object_classes.end())
		{
			// TODO: error?
			return;
		}

		const ObjectClassData& class_data = class_data_it->second;
		const ObjectPropertyLookup property_lookup = internal_find_property_info(class_data, property_name);
		if (property_lookup.is_valid() == false)
		{
			// TODO: error?
			return;
		}

		// FIXME: properties should store an index so we don't have to do this lookup!
		const MemberFunctionData& property_setter = property_lookup.class_data->methods.find(property_lookup.property_info->setter)->second;

		invoke_method(object, property_lookup.property_info->setter, property_setter, VariantArgumentList{ &value, 1 });
	}

	ObjectSystem::ObjectSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Core::ObjectSystem(core)
	{
	}

	bool ObjectSystem::initialize()
	{
		// Register base Object type
		register_object_class<Object>();

		// TODO: other types!
		// TODO2: engine types should be marked as immutable somehow (i.e throw error if client tries to mess with them!)

		return true;
	}

	Vadon::Core::ObjectClassData* ObjectSystem::internal_register_object_class(ClassRegistryInfo class_info, ErasedObjectFactoryFunction factory)
	{
		// First make sure the ID is unique
		if (m_object_classes.find(std::string(class_info.id)) != m_object_classes.end())
		{
			error(std::format("Object system error: object type \"{}\" already registered!\n", class_info.id));
			return nullptr;
		}

		// If no pretty name is provided, use class ID
		// TODO: enforce requirements on name?
		// TODO2: deduplicate names?
		ObjectClassData& new_class_data = m_object_classes.emplace(class_info.id, m_engine_core).first->second;
		
		new_class_data.class_info.id = class_info.id;
		new_class_data.class_info.base_id = class_info.base_id;
		new_class_data.class_info.name = get_pretty_name(class_info.pretty_name, class_info.id);

		if (class_info.id != class_info.base_id)
		{
			auto base_class_it = m_object_classes.find(std::string(class_info.base_id));
			if (base_class_it == m_object_classes.end())
			{
				error(std::format("Object system error: object type \"{}\" has unregistered base \"{}\"!\n", class_info.id, class_info.base_id));
				return nullptr;
			}
			new_class_data.base_data = &base_class_it->second;

			// Add reverse lookup
			new_class_data.base_data->subclasses.push_back(&new_class_data);
		}

		new_class_data.factory = factory;

		return &new_class_data;
	}

	ObjectSystem::ObjectPropertyLookup ObjectSystem::internal_find_property_info(const ObjectClassData& class_data, std::string_view property_name) const
	{
		ObjectSystem::ObjectPropertyLookup result;

		const ObjectClassData* current_class_data = &class_data;
		while (current_class_data != nullptr)
		{
			auto property_info_it = current_class_data->properties.find(std::string(property_name));
			if (property_info_it != current_class_data->properties.end())
			{
				result.class_data = current_class_data;
				result.property_info = &property_info_it->second;

				return result;
			}
			current_class_data = current_class_data->base_data;
		}

		return result;
	}

	const MemberFunctionData* ObjectSystem::internal_find_method(const ObjectClassData& class_data, std::string_view method_name) const
	{
		const ObjectClassData* current_class_data = &class_data;
		while (current_class_data != nullptr)
		{
			auto method_it = current_class_data->methods.find(std::string(method_name));
			if (method_it != current_class_data->methods.end())
			{
				return &method_it->second;
			}
			current_class_data = current_class_data->base_data;
		}

		return nullptr;
	}

	Variant ObjectSystem::invoke_method(Object& object, std::string_view method_name, const MemberFunctionData& method_data, VariantArgumentList args) const
	{
		// Validation
		if (method_data.argument_types.size() != args.size())
		{
			// FIXME: have to do this because this is a const function, need to revise!
			m_engine_core.get_logger().error(std::format("Mismatch in argument count for member function \"{}\"!\n", method_name));
			return Variant();
		}

		for (size_t current_argument_index = 0; current_argument_index < method_data.argument_types.size(); ++current_argument_index)
		{
			if (method_data.argument_types[current_argument_index] != args[current_argument_index].index())
			{
				// FIXME: have to do this because this is a const function, need to revise!
				m_engine_core.get_logger().error(std::format("Mismatch in argument type for member function \"{}\"!\n", method_name));
				return Variant();
			}
		}

		return method_data.function(&object, args);
	}

	bool ObjectSystem::is_instance_of(Object& object, std::string_view class_id) const
	{
		std::string_view object_class_id = object.get_class_id();
		if (object_class_id == class_id)
		{
			return true;
		}

		auto object_class_data_it = m_object_classes.find(std::string(object.get_class_id()));
		auto target_class_data_it = m_object_classes.find(std::string(class_id));
		if ((object_class_data_it == m_object_classes.end()) || (target_class_data_it == m_object_classes.end()))
		{
			// TODO: error?
			return false;
		}

		const ObjectClassData* object_class_data = &object_class_data_it->second;
		const ObjectClassData* target_class_data = &target_class_data_it->second;
		while (object_class_data != nullptr)
		{
			if (object_class_data->base_data == target_class_data)
			{
				// Requested type is base class of this instance
				return true;
			}

			// Check further up the hierarchy
			object_class_data = object_class_data->base_data;
		}

		// Types are in different branches of Object hierarchy
		return false;
	}
}