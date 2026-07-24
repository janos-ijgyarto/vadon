#ifndef VADON_MODEL_RESOURCE_RESOURCE_HPP
#define VADON_MODEL_RESOURCE_RESOURCE_HPP
#include <Vadon/Foundation/Model/Resource/Resource.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/TypeInfo/TypeErasure.hpp>
namespace Vadon::Model
{
	VADON_DECLARE_TYPED_POOL_HANDLE(ResourceBase, ResourceHandle);
	using ResourceID = ::Vadon::Foundation::UUID;
		
	struct ResourceInfo
	{
		ResourceID id;
		Vadon::Utilities::TypeID type_id = Vadon::Utilities::TypeID::INVALID;

		bool is_valid() const { return id.is_valid(); }
	};

	// TODO: revise as class to restrict access to base members?
	struct Resource
	{
		VADON_DECLARE_MEMBER_UUID(name, "27c87325-75f7-4c32-b309-0dd56f8cc5d9");

		ResourceID id;
		std::string name;

		virtual ~Resource() {}
	};

	template<typename T>
	struct TypedResourceID : public ResourceID
	{
		using _ResourceType = T;
		using _TypedID = TypedResourceID<T>;

		TypedResourceID<T>& operator=(const ResourceID& id) { data = id.data; return *this; }

		ResourceID to_resource_id() const { return ResourceID{ .data = this->data }; }
		static TypedResourceID<T> from_resource_id(ResourceID id) { _TypedID typed_id; memcpy(typed_id.data, id.data, ::Vadon::Foundation::UUID::c_uuid_width); return typed_id; }

		ResourceID& as_resource_id() { return *this; }
	};

	template<typename T>
	struct TypedResourceHandle : public ResourceHandle
	{
		using _ResourceType = T;
		using _TypedHandle = TypedResourceHandle<T>;

		TypedResourceHandle<T>& operator=(const ResourceHandle& h) { handle = h.handle; return *this; }

		ResourceHandle to_resource_handle() const { return ResourceHandle{ .handle = this->handle }; }
		static TypedResourceHandle<T> from_resource_handle(ResourceHandle h) { _TypedHandle typed_handle; typed_handle.handle = h.handle; return typed_handle; }

		uint64_t to_uint() const { return this->handle.to_uint(); }
	};

	template<typename T>
	concept is_resource_id = std::is_base_of_v<Vadon::Model::ResourceID, T> && (std::is_same_v<Vadon::Model::ResourceID, T> == false);

	template<typename T>
	concept is_resource_handle = std::is_base_of_v<Vadon::Model::ResourceHandle, T> && (std::is_same_v<Vadon::Model::ResourceHandle, T> == false);
}

namespace Vadon::Utilities
{
	template<Vadon::Model::is_resource_id T>
	struct TypeErasureTrait<T>
	{
		static constexpr ::Vadon::Foundation::UUID get_erased_type_uuid()
		{
			return Vadon::Utilities::string_to_uuid(::Vadon::Foundation::ResourceIDSchema::c_type_uuid);
		}

		static constexpr size_t get_nested_type_count()
		{
			// Resource ID "header" + the resource type itself
			return 2;
		}

		static constexpr void add_erased_type_uuid(::Vadon::Foundation::UUID* uuid_array)
		{
			*uuid_array = get_erased_type_uuid();

			++uuid_array;

			*uuid_array = TypeRegistryTrait<typename T::_ResourceType>::get_type_uuid();
		}

		static constexpr auto get_type_list()
		{
			std::array<::Vadon::Foundation::UUID, get_nested_type_count()> type_list;
			add_erased_type_uuid(type_list.data());
			return type_list;
		}
	};

	template<Vadon::Model::is_resource_id T>
	struct VariantTypeTrait<T>
	{
		static Variant to_variant(const T& value)
		{
			return VariantTypeTrait<::Vadon::Foundation::UUID>::to_variant(value);
		}

		static T from_variant(const Variant& variant)
		{
			return T(VariantTypeTrait<::Vadon::Foundation::UUID>::from_variant(variant));
		}
	};
}

VADON_REGISTER_TYPE_UUID(Vadon::Model::Resource, ::Vadon::Foundation::ResourceSchema::c_type_uuid.string);
VADON_REGISTER_TYPE_UUID(::Vadon::Foundation::ResourceIDSchema, ::Vadon::Foundation::ResourceIDSchema::c_type_uuid.string);

#define VADON_MODEL_DECLARE_TYPED_RESOURCE_ID(_resource, _name) using _name = Vadon::Model::TypedResourceID<_resource>
#define VADON_MODEL_DECLARE_TYPED_RESOURCE_HANDLE(_resource, _name) using _name = Vadon::Model::TypedResourceHandle<_resource>
#define VADON_MODEL_DECLARE_TYPED_RESOURCE_REFERENCES(_resource, _id_name, _handle_name) VADON_MODEL_DECLARE_TYPED_RESOURCE_ID(_resource, _id_name); \
																						VADON_MODEL_DECLARE_TYPED_RESOURCE_HANDLE(_resource, _handle_name)
			
#endif