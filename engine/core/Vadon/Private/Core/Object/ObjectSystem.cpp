#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Core/Object/ObjectSystem.hpp>

#include <format>
#include <deque>

namespace Vadon::Private::Core
{
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

	bool ObjectSystem::ObjectClassData::add_property(ObjectProperty property)
	{
		if (has_property(property.name) == true)
		{
			engine_core.get_logger().error(std::format("Class \"{}\" already has property registered with name \"{}\"!\n", class_info.name, property.name));
			return false;
		}

		// FIXME: find some way to deduplicate the steps below?
		// Validate getter
		{
			auto getter_it = methods.find(std::string(property.getter));
			if (getter_it == methods.end())
			{
				engine_core.get_logger().error(std::format("Class \"{}\" has no getter for property \"{}\" named \"{}\"!\n", class_info.name, property.name, property.getter));
				return false;
			}

			// TODO: further restrictions to make sure getter is of the correct format (i.e no args)
		}

		// Validate setter
		{
			auto setter_it = methods.find(std::string(property.setter));
			if (setter_it == methods.end())
			{
				engine_core.get_logger().error(std::format("Class has no setter for property \"{}\" named \"{}\"!\n", class_info.name, property.setter));
				return false;
			}

			// TODO: further restrictions to make sure setter is of the correct format (i.e one argument, matches property type)
		}

		properties.emplace(std::string(property.name), property);
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

	Variant ObjectSystem::ObjectClassData::invoke_method(Object& object, std::string_view name, VariantArgumentList args) const
	{
		auto method_it = methods.find(std::string(name));
		if (method_it == methods.end())
		{
			engine_core.get_logger().error(std::format("No member function registered with name \"{}\"!\n", name));
			return Variant();
		}

		const MemberFunctionData& method_data = method_it->second;

		// Validation
		if (method_data.argument_types.size() != args.size())
		{
			engine_core.get_logger().error(std::format("Mismatch in argument count for member function \"{}\"!\n", name));
			return Variant();
		}

		for (size_t current_argument_index = 0; current_argument_index < method_data.argument_types.size(); ++current_argument_index)
		{
			if (method_data.argument_types[current_argument_index] != args[current_argument_index].index())
			{
				engine_core.get_logger().error(std::format("Mismatch in argument type for member function \"{}\"!\n", name));
				return Variant();
			}
		}

		return method_data.function(&object, args);
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

	ObjectPropertyList ObjectSystem::get_class_properties(std::string_view class_id, bool recursive) const
	{
		ObjectPropertyList property_list;

		auto class_data_it = m_object_classes.find(std::string(class_id));
		if (class_data_it == m_object_classes.end())
		{
			// TODO: error?
			return property_list;
		}

		// Start with own properties
		const ObjectClassData& class_data = class_data_it->second;
		internal_get_class_properties(class_data, property_list);

		if (recursive == true)
		{
			// Get properties of the rest of the inheritance chain
			const ObjectClassData* current_base_data = class_data.base_data;
			while (current_base_data != nullptr)
			{
				internal_get_class_properties(*current_base_data, property_list);
				current_base_data = current_base_data->base_data;
			}
		}
		return property_list;
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
		auto property_it = class_data.properties.find(std::string(property_name));
		if(property_it == class_data.properties.end())
		{
			// TODO: error?
			return Variant();
		}

		const ObjectProperty& property_data = property_it->second;
		return class_data.invoke_method(object, property_data.getter);
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
		auto property_it = class_data.properties.find(std::string(property_name));
		if (property_it == class_data.properties.end())
		{
			// TODO: error?
			return;
		}

		const ObjectProperty& property_data = property_it->second;
		class_data.invoke_method(object, property_data.getter, VariantArgumentList{ &value, 1 });
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
		new_class_data.class_info.name = class_info.pretty_name.empty() ? class_info.id : class_info.pretty_name;

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

	void ObjectSystem::internal_get_class_properties(const ObjectClassData& class_data, ObjectPropertyList& properties) const
	{
		properties.reserve(properties.size() + class_data.properties.size());
		for (const auto& current_property : class_data.properties)
		{
			properties.push_back(current_property.second);
		}
	}
}