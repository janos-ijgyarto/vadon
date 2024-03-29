#ifndef VADON_CORE_OBJECT_OBJECTSYSTEM_HPP
#define VADON_CORE_OBJECT_OBJECTSYSTEM_HPP
#include <Vadon/Core/CoreModule.hpp>

#include <Vadon/Core/Object/ClassInfo.hpp>
#include <Vadon/Core/Object/Object.hpp>
#include <Vadon/Core/Object/Property.hpp>

#include <functional>
namespace Vadon::Core
{
	struct ObjectProperty;

	class ObjectSystem : public CoreSystem<ObjectSystem>
	{
	public:
		virtual Object* create_object(std::string_view class_id) = 0;

		template<typename T>
		T* get_object_as(Object& object) const
		{
			static_assert(std::is_base_of_v<Object, T> == true, "Object System can only use RTTI with subclasses of Object!");
			if (is_instance_of(object, T::static_class_id()) == false)
			{
				return nullptr;
			}

			return static_cast<T*>(&object);
		}

		template<typename T>
		bool register_object_class(std::string_view pretty_name = "")
		{
			static_assert(std::is_base_of_v<Object, T> == true, "Object System only allows registering factories for subclasses of Object!");
			ObjectClassData* class_data = internal_register_object_class(std::move(ClassRegistryInfo{.id = T::static_class_id(), .base_id = T::_Base::static_class_id(), .pretty_name = pretty_name }), erase_object_factory_function<T>());
			if (class_data == nullptr)
			{
				return false;
			}

			T::initialize_class(*class_data);
			return true;
		}

		// TODO: use std::expected so we can check for failure?
		template<typename T>
		ObjectClassInfo get_class_info() const
		{
			static_assert(std::is_base_of_v<Object, T> == true, "Object System can only provide class info for subclasses of Object!");
			return get_class_info(T::static_class_id());
		}

		virtual ObjectClassInfo get_class_info(std::string_view class_id) const = 0;

		virtual ObjectClassInfoList get_class_list() const = 0;

		template<typename T>
		ObjectClassInfoList get_subclass_hierarchy() const
		{
			static_assert(std::is_base_of_v<Object, T> == true, "Object System can only provide class info for subclasses of Object!");
			return get_subclass_hierarchy(T::static_class_id());
		}

		virtual ObjectClassInfoList get_subclass_hierarchy(std::string_view class_id) const = 0;

		virtual ObjectPropertyInfoList get_class_properties(std::string_view class_id, bool recursive = true) const = 0;

		// FIXME: implement some kind of ID system so we can skip string lookups!
		// FIXME2: should object itself have a getter/setter for this?
		virtual ObjectPropertyList get_object_properties(Object& object, bool recursive = true) const = 0;
		virtual Variant get_property(Object& object, std::string_view property_name) const = 0;
		virtual void set_property(Object& object, std::string_view property_name, Variant value) = 0;
	protected:
		ObjectSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}

		struct ClassRegistryInfo
		{
			std::string_view id;
			std::string_view base_id;
			std::string_view pretty_name;
		};

		using ErasedObjectFactoryFunction = Object*(*)(EngineCoreInterface&);

		template <typename T>
		ErasedObjectFactoryFunction erase_object_factory_function()
		{
			return +[](EngineCoreInterface& core) { return static_cast<Object*>(new T(core)); };
		}

		virtual ObjectClassData* internal_register_object_class(ClassRegistryInfo class_info, ErasedObjectFactoryFunction factory) = 0;
		virtual bool is_instance_of(Object& object, std::string_view class_id) const = 0;
	};
}
#endif