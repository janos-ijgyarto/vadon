#ifndef VADON_UTILITIES_TYPE_REGISTRY_REGISTRY_HPP
#define VADON_UTILITIES_TYPE_REGISTRY_REGISTRY_HPP
#include <Vadon/Common.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
#include <Vadon/Utilities/TypeInfo/TypeName.hpp>

#include <Vadon/Utilities/TypeInfo/Registry/MemberBind.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Property.hpp>

#include <unordered_map>

namespace Vadon::Utilities
{
	class TypeRegistry
	{
	public:
		using TypeID = uint32_t;
		static constexpr TypeID c_invalid_type_id = 0;

		template<typename T>
		static void register_type()
		{
			internal_register_type(Vadon::Utilities::TypeName<T>::trimmed());
		}

		template<typename T>
		static TypeID get_type_id()
		{
			static TypeID type_id = get_type_id(Vadon::Utilities::TypeName<T>::trimmed());
			return type_id;
		}

		VADONCOMMON_API static TypeID get_type_id(std::string_view type_name);

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
		VADONCOMMON_API static TypeInfo get_type_info(uint32_t type_id);

		// TODO: use std::expected so we can check for failure?
		template<typename T>
		static PropertyInfoList get_type_properties()
		{
			return get_type_properties(get_type_id<T>());
		}

		// TODO: use std::expected so we can check for failure?
		VADONCOMMON_API static PropertyInfoList get_type_properties(uint32_t type_id);

		template<typename T>
		static PropertyList get_properties(T& object)
		{
			return get_type_properties(&object, get_type_id<T>());
		}

		VADONCOMMON_API static PropertyList get_properties(void* object, uint32_t type_id);

		VADONCOMMON_API static Variant get_property(void* object, uint32_t type_id, std::string_view property_name);
		VADONCOMMON_API static void set_property(void* object, uint32_t type_id, std::string_view property_name, const Variant& value);
	protected:
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

		static VADONCOMMON_API void internal_register_type(std::string_view type_name);
		static VADONCOMMON_API bool internal_add_property(TypeID type_id, std::string_view name, MemberVariableBindBase property_bind);
		static VADONCOMMON_API bool internal_bind_method(TypeID type_id, std::string_view name, MemberFunctionBind method_bind);

		// FIXME: hide via PIMPL?
		std::unordered_map<std::string, uint32_t> m_id_lookup;
		uint32_t m_id_counter = 1;

		std::unordered_map<uint32_t, TypeData> m_type_lookup;
	};
}
#endif