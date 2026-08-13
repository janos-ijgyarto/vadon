#ifndef VADON_UTILITIES_DATA_OBJECT_HPP
#define VADON_UTILITIES_DATA_OBJECT_HPP
#include <Vadon/Utilities/Debugging/Assert.hpp>
#include <Vadon/Utilities/TypeInfo/Registry.hpp>

#include <Vadon/Foundation/TypeInfo/Object.hpp>

namespace Vadon::Utilities
{
	class ObjectWrapper
	{
	public:
		explicit ObjectWrapper(TypeID type = TypeID::INVALID, void* data = nullptr)
			: m_type(type)
			, m_data(data)
		{
		}

		ObjectWrapper(const ObjectWrapper& other)
			: ObjectWrapper(other.m_type, other.m_data)
		{
		}

		ObjectWrapper& operator=(const ObjectWrapper& other)
		{
			m_type = other.m_type;
			m_data = other.m_data;
			return *this;
		}

		TypeID get_type() const { return m_type; }
		void* get_data() const { return m_data; }

		bool is_valid() const { return (m_type != TypeID::INVALID) && (m_data != nullptr); }
	protected:
		TypeID m_type;
		void* m_data = nullptr;
	};

	VADON_REGISTER_TYPE_UUID(ObjectWrapper, ::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid.string);

	template<typename T>
	class TypedObjectWrapper : public ObjectWrapper
	{
	public:
		using _WrappedType = T;

		explicit TypedObjectWrapper(void* data = nullptr)
			: ObjectWrapper(TypeRegistry::get_type_id<T>(), data)
		{
		}

		// TODO: also implement versions which accept a subclass of the type!
		explicit TypedObjectWrapper(const ObjectWrapper& other)
			: TypedObjectWrapper(other.get_data())
		{
			VADON_ASSERT(other.get_type() == TypeRegistry::get_type_id<T>(), "Object type is not compatible!");
		}

		TypedObjectWrapper(const TypedObjectWrapper<T>& other)
			: TypedObjectWrapper(other.m_data)
		{
		}

		TypedObjectWrapper<T>& operator=(const TypedObjectWrapper& other)
		{
			m_data = other.m_data;
			return *this;
		}

		TypedObjectWrapper<T>& operator=(const ObjectWrapper& other)
		{
			VADON_ASSERT(other.get_type() == m_type, "Object type is not compatible!");
			m_data = other.get_data();
			return *this;
		}

		T& operator*() { return *static_cast<T*>(m_data); }
		const T& operator*() const { return *static_cast<const T*>(m_data); }

		T* operator->() { return static_cast<T*>(m_data); }
		const T* operator->() const { return static_cast<const T*>(m_data); }
	};

	class DataObject
	{
	public:
		DataObject(TypeID type_id = TypeID::INVALID)
			: m_type_id(type_id)
		{}

		TypeID get_type_id() const { return m_type_id; }
		const VariantDictionary& get_properties() const { return m_properties; }

		VADONCOMMON_API Variant get_property(const PropertyUUID& property_id);
		VADONCOMMON_API void set_property(const PropertyUUID& property_id, const Variant& value);

		VADONCOMMON_API VariantDictionary export_data() const;
		VADONCOMMON_API bool import_data(const VariantDictionary& data);
	private:
		TypeID m_type_id;
		VariantDictionary m_properties;
	};

	VADON_REGISTER_TYPE_UUID(Vadon::Utilities::DataObject, ::Vadon::Foundation::DataObjectSchema::c_type_uuid.string);

	template<>
	struct VariantTypeTrait<DataObject>
	{
		static Variant to_variant(const DataObject& value)
		{
			return Box(value.export_data());
		}

		static DataObject from_variant(const Variant& variant)
		{
			const VariantDictionary& object_dictionary = *std::get<BoxedVariantDictionary>(variant);
			DataObject data_object;

			if (data_object.import_data(object_dictionary) == false)
			{
				return DataObject{};
			}

			return data_object;
		}
	};

	class Serializer;

	class ObjectSerializer
	{
	public:
		VADONCOMMON_API static bool serialize_object(Serializer& serializer, VariantDictionary& object_dictionary);
		VADONCOMMON_API static bool serialize_object_properties(Serializer& serializer, TypeID object_type, VariantDictionary& object_properties);

		VADONCOMMON_API static bool load_object_data(ObjectWrapper& object, const VariantDictionary& data);
		VADONCOMMON_API static bool load_object_property_data(ObjectWrapper& object, const VariantDictionary& property_data);

		VADONCOMMON_API static bool store_object_data(const ObjectWrapper& object, VariantDictionary& data);
		VADONCOMMON_API static bool store_object_property_data(const ObjectWrapper& object, VariantDictionary& property_data);
	};

	template<>
	struct VariantTypeTrait<ObjectWrapper>
	{
		static Variant to_variant(const ObjectWrapper& value)
		{
			VariantDictionary object_dictionary;
			if (ObjectSerializer::store_object_data(value, object_dictionary) == false)
			{
				return Box(VariantDictionary{});
			}

			return Box(object_dictionary);
		}

		static ObjectWrapper from_variant(const Variant& variant)
		{
			const VariantDictionary& object_dictionary = *std::get<BoxedVariantDictionary>(variant);
			
			ObjectWrapper object;
			if(ObjectSerializer::load_object_data(object, object_dictionary) == false)
			{
				return ObjectWrapper();
			}

			return object;
		}
	};

	template<typename T>
	struct ObjectVariantTypeTrait
	{
		static Variant to_variant(const T& value)
		{
			ObjectWrapper obj_wrapper(TypeRegistry::get_type_id<T>(), const_cast<T*>(&value));
			VariantDictionary properties_dictionary;
			if (ObjectSerializer::store_object_property_data(obj_wrapper, properties_dictionary) == false)
			{
				VADON_ERROR("Failed to store property data!");
				return Box(VariantDictionary{});
			}
			return Box(properties_dictionary);
		}

		static T from_variant(const Variant& variant)
		{
			T object;
			ObjectWrapper obj_wrapper(TypeRegistry::get_type_id<T>(), &object);
			const VariantDictionary& properties_dictionary = *std::get<BoxedVariantDictionary>(variant);
			if (ObjectSerializer::load_object_property_data(obj_wrapper, properties_dictionary) == false)
			{
				VADON_ERROR("Failed to load property data!");
				return T{};
			}

			return object;
		}
	};

	template<typename T>
	using as_typed_object_wrapper = TypedObjectWrapper<typename T::_WrappedType>;

	template<typename T>
	concept is_typed_object_wrapper = std::is_same_v<T, as_typed_object_wrapper<T>>;

	template<is_typed_object_wrapper T>
	struct VariantTypeTrait<T>
	{
		static Variant to_variant(const T& value)
		{
			VariantDictionary object_dictionary;
			ObjectWrapper generic_wrapper(value.get_type(), value.get_data());
			if (ObjectSerializer::store_object_data(generic_wrapper, object_dictionary) == false)
			{
				VADON_ERROR("Failed to store object data!");
				return Box(VariantDictionary{});
			}
			return Box(object_dictionary);
		}

		static T from_variant(const Variant& variant)
		{
			ObjectWrapper object;
			const VariantDictionary& properties_dictionary = *std::get<BoxedVariantDictionary>(variant);
			if (ObjectSerializer::load_object_data(object, properties_dictionary) == false)
			{
				VADON_ERROR("Failed to load object data!");
				return T{};
			}

			return T(object);
		}
	};
}
#define VADON_DEFINE_OBJECT_VARIANT_TYPE_TRAIT(_type) template<>\
struct Vadon::Utilities::VariantTypeTrait<_type>\
{\
	static Vadon::Utilities::Variant to_variant(const _type& value)\
	{\
		return ObjectVariantTypeTrait<_type>::to_variant(value);\
	}\
	static _type from_variant(const Vadon::Utilities::Variant& variant)\
	{\
		return ObjectVariantTypeTrait<_type>::from_variant(variant);\
	}\
}
#endif