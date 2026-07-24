#ifndef VADON_UTILITIES_TYPEINFO_REGISTRY_HPP
#define VADON_UTILITIES_TYPEINFO_REGISTRY_HPP
#include <Vadon/Common.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBindBase.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>

namespace Vadon::Utilities
{
	class ObjectWrapper;

	class TypeRegistry
	{
	public:
		using ObjectFactoryFunction = void*(*)();
		using ObjectDestructorFunction = void(*)(const void*);

		// TODO: use allocator API to ensure that we don't just naively heap-allocate!
		struct ObjectFactory
		{
			ObjectFactoryFunction factory_function = nullptr;
			ObjectDestructorFunction destructor_function = nullptr;

			template<typename T>
			static void* default_factory_function()
			{
				return new T();
			}

			template<typename T>
			static void default_destructor_function(const void* object)
			{
				delete static_cast<const T*>(object);
			}

			template<typename T>
			static ObjectFactory get_default_factory()
			{
				return ObjectFactory{ .factory_function = &ObjectFactory::default_factory_function<T>,
					.destructor_function = &ObjectFactory::default_destructor_function<T> };
			}
		};

		TypeRegistry();

		bool initialize();

		template<typename T, typename Base = T>
		static void register_type()
		{
			if constexpr (std::is_same_v<T, Base>)
			{
				internal_register_type(TypeRegistryTrait<T>::get_type_uuid(), sizeof(T), alignof(T));
			}
			else
			{
				// NOTE: calling it this way ensures that Base was registered first
				internal_register_type(TypeRegistryTrait<T>::get_type_uuid(), sizeof(T), alignof(T), get_type_id<Base>());
			}
		}

		template<typename T>
		static void register_type_factory(ObjectFactory factory = ObjectFactory::get_default_factory())
		{
			internal_register_type_factory(get_type_id<T>(), factory);
		}

		VADONCOMMON_API static ObjectWrapper create_object(TypeID type_id);
		VADONCOMMON_API static void destroy_object(const ObjectWrapper& object_instance);

		template<typename T>
		static TypeID get_type_id()
		{
			static TypeID type_id = get_type_id(TypeRegistryTrait<T>::get_type_uuid());
			return type_id;
		}

		VADONCOMMON_API static TypeID get_type_id(const TypeUUID& type_uuid);

		VADONCOMMON_API static bool is_base_of(const TypeUUID& base_uuid, const TypeUUID& type_uuid);
		VADONCOMMON_API static bool is_base_of(TypeID base_id, TypeID type_id);

		template<typename T>
		static bool add_property(const PropertyUUID& property_uuid, MemberVariableBindBase property_bind)
		{
			return internal_add_property(get_type_id<T>(), property_uuid, std::move(property_bind));
		}

		// FIXME: extract type from member function?
		template <typename T, auto Ptr>
		static bool bind_method(const MemberFunctionUUID& method_uuid)
		{
			return internal_bind_method(get_type_id<T>(), method_uuid, std::move(create_member_function_bind<Ptr>()));
		}

		// TODO: use std::expected so we can check for failure?
		template<typename T>
		static ::Vadon::Foundation::TypeInfo get_type_info()
		{
			return get_type_info(get_type_id<T>());
		}

		// TODO: use std::expected so we can check for failure?
		VADONCOMMON_API static ::Vadon::Foundation::TypeInfo get_type_info(TypeID type_id);

		// TODO: use std::expected so we can check for failure?
		template<typename T>
		static std::vector<TypeID> get_subclass_list()
		{
			return get_subclass_list(get_type_id<T>());
		}

		// TODO: use std::expected so we can check for failure?
		VADONCOMMON_API static std::vector<TypeID> get_subclass_list(TypeID type_id);

		// TODO: use std::expected so we can check for failure?
		template<typename T>
		static PropertyInfoList get_type_properties()
		{
			return get_type_properties(get_type_id<T>());
		}

		// TODO: use std::expected so we can check for failure?
		VADONCOMMON_API static PropertyInfoList get_type_properties(TypeID type_id);

		// TODO: add non-recursive version?
		template<typename T>
		static PropertyList get_properties(T& object)
		{
			return get_properties(&object, get_type_id<T>());
		}

		VADONCOMMON_API static PropertyList get_properties(void* object, TypeID type_id);

		VADONCOMMON_API static PropertyInfo get_property_info(TypeID type_id, const PropertyUUID& property_uuid);

		VADONCOMMON_API static Variant get_property(void* object, TypeID type_id, const PropertyUUID& property_uuid);
		VADONCOMMON_API static void set_property(void* object, TypeID type_id, const PropertyUUID& property_uuid, const Variant& value);

		VADONCOMMON_API static void* get_property_member_address(void* object, TypeID type_id, const PropertyUUID& property_uuid);

		VADONCOMMON_API static void apply_property_values(void* object, TypeID type_id, const PropertyList& properties);

		VADONCOMMON_API static std::vector<TypeUUID> get_all_registered_types();
	private:
		struct PropertyData
		{
			::Vadon::Foundation::Property info;
			MemberVariableBindBase member_bind;
		};

		struct TypeData
		{
			// TODO: allow storing readable name (if provided by client code)
			// Add utility function to print the name if available
			::Vadon::Foundation::TypeInfo info;
			TypeID base_id;
			bool base_type = false;

			ObjectFactory object_factory;

			std::unordered_map<MemberFunctionUUID, MemberFunctionBind> methods;
			std::unordered_map<PropertyUUID, PropertyData> properties;

			bool bind_method(const MemberFunctionUUID& method_uuid, MemberFunctionBind method);
			bool add_property(const PropertyUUID& property_uuid, const ::Vadon::Foundation::Property& property_info, MemberVariableBindBase member_bind);

			bool has_method(const MemberFunctionUUID& method_uuid) const;
			bool has_property(const PropertyUUID& property_uuid) const;
		};

		static VADONCOMMON_API void internal_register_type(const TypeUUID& type_uuid, size_t size, size_t alignment, TypeID base_id = TypeID::INVALID);
		static VADONCOMMON_API void internal_register_type_factory(TypeID type_id, ObjectFactory factory);

		static VADONCOMMON_API bool internal_add_property(TypeID type_id, const PropertyUUID& property_uuid, MemberVariableBindBase property_bind);
		static VADONCOMMON_API bool internal_bind_method(TypeID type_id, const MemberFunctionUUID& method_uuid, MemberFunctionBind method_bind);

		void register_type_with_base(TypeID type_id, TypeData& data, TypeID base_id);

		bool has_method(TypeID type_id, const MemberFunctionUUID& method_uuid) const;
		bool has_property(TypeID type_id, const PropertyUUID& property_uuid) const;

		void internal_get_type_properties(TypeID type_id, PropertyInfoList& property_list) const;
		void internal_get_properties(void* object, TypeID type_id, PropertyList& property_list) const;

		const PropertyData* internal_find_property(const TypeData& type_data, const PropertyUUID& property_uuid) const;
		PropertyData* internal_find_property(const TypeData& type_data, const PropertyUUID& property_uuid) { return const_cast<PropertyData*>(std::as_const(*this).internal_find_property(type_data, property_uuid)); }

		void internal_apply_property_value(const TypeData& type_data, void* object, const PropertyUUID& property_uuid, const Variant& value);

		// FIXME: hide via PIMPL?
		bool m_initialized = false;

		std::unordered_map<TypeUUID, TypeID> m_id_lookup;
		std::underlying_type_t<TypeID> m_id_counter = 1;

		// FIXME: use vector to improve lookup times?
		std::unordered_map<TypeID, TypeData> m_type_lookup;
	};
}
#endif