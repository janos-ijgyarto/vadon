#ifndef VADON_UTILITIES_TYPEINFO_REGISTRY_HPP
#define VADON_UTILITIES_TYPEINFO_REGISTRY_HPP
#include <Vadon/Common.hpp>
#include <Vadon/Utilities/TypeInfo/TypeName.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/FunctionBindBase.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBindBase.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>

namespace Vadon::Utilities
{
	class TypeRegistry
	{
	public:
		template<typename T, typename Base = T>
		static void register_type(std::string_view hint_string = std::string_view())
		{
			if constexpr (std::is_same_v<T, Base>)
			{
				internal_register_type(Vadon::Utilities::TypeName<T>::trimmed(), hint_string, sizeof(T), alignof(T));
			}
			else
			{
				internal_register_type(Vadon::Utilities::TypeName<T>::trimmed(), hint_string, sizeof(T), alignof(T), get_type_id<Base>());
			}
		}

		template<typename T>
		static TypeID get_type_id()
		{
			static TypeID type_id = get_type_id(Vadon::Utilities::TypeName<T>::trimmed());
			return type_id;
		}

		VADONCOMMON_API static TypeID get_type_id(std::string_view type_name);

		VADONCOMMON_API static bool is_base_of(TypeID base_id, TypeID type_id);

		// TODO: property hints?
		template<typename T>
		static bool add_property(std::string_view name, MemberVariableBindBase property_bind)
		{
			return internal_add_property(get_type_id<T>(), name, std::move(property_bind));
		}

		// FIXME: extract type from member function?
		template <typename T, auto Ptr>
		static bool bind_method(std::string_view name)
		{
			return internal_bind_method(get_type_id<T>(), name, std::move(create_member_function_bind<Ptr>()));
		}

		// TODO: use std::expected so we can check for failure?
		template<typename T>
		static TypeInfo get_type_info()
		{
			return get_type_info(get_type_id<T>());
		}

		// TODO: use std::expected so we can check for failure?
		VADONCOMMON_API static TypeInfo get_type_info(TypeID type_id);

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

		VADONCOMMON_API static Variant get_property(void* object, TypeID type_id, std::string_view property_name);
		VADONCOMMON_API static void set_property(void* object, TypeID type_id, std::string_view property_name, const Variant& value);

		VADONCOMMON_API static void apply_property_values(void* object, TypeID type_id, const PropertyList& properties);
	private:
		struct TypeData
		{
			TypeInfo info;

			// FIXME: implement more efficient lookups and bookkeeping!
			std::unordered_map<std::string, MemberFunctionBind> methods;
			std::unordered_map<std::string, MemberVariableBindBase> properties;

			bool bind_method(std::string_view name, MemberFunctionBind method);
			bool add_property(std::string_view name, MemberVariableBindBase property);

			bool has_method(std::string_view name) const;
			bool has_property(std::string_view name) const;
		};

		static VADONCOMMON_API void internal_register_type(std::string_view type_name, std::string_view hint_string, size_t size, size_t alignment, TypeID base_type_id = TypeID::INVALID);
		static VADONCOMMON_API bool internal_add_property(TypeID type_id, std::string_view name, MemberVariableBindBase property_bind);
		static VADONCOMMON_API bool internal_bind_method(TypeID type_id, std::string_view name, MemberFunctionBind method_bind);
		
		void register_type_with_base(TypeID type_id, TypeData& data, TypeID base_id);

		bool has_method(TypeID type_id, std::string_view method_name) const;
		bool has_property(TypeID type_id, std::string_view property_name) const;

		void internal_get_type_properties(TypeID type_id, PropertyInfoList& property_list) const;
		void internal_get_properties(void* object, TypeID type_id, PropertyList& property_list) const;

		const MemberVariableBindBase* internal_find_property(const TypeData& type_data, std::string_view name) const;
		void internal_apply_property_value(const TypeData& type_data, void* object, std::string_view property_name, const Variant& value);

		// FIXME: hide via PIMPL?
		std::unordered_map<std::string, TypeID> m_id_lookup;
		std::underlying_type_t<TypeID> m_id_counter = 1;

		// FIXME: use vector to improve lookup times?
		std::unordered_map<TypeID, TypeData> m_type_lookup;
	};
}
#endif