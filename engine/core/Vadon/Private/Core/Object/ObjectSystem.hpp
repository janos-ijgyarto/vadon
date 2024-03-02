#ifndef VADON_PRIVATE_CORE_OBJECT_OBJECTSYSTEM_HPP
#define VADON_PRIVATE_CORE_OBJECT_OBJECTSYSTEM_HPP
#include <Vadon/Core/Object/ObjectSystem.hpp>
#include <Vadon/Core/Object/ClassData.hpp>

#include <Vadon/Private/Core/Object/FunctionBind.hpp>
#include <Vadon/Private/Core/Object/Object.hpp>
#include <Vadon/Private/Core/Object/Variant.hpp>

namespace Vadon::Private::Core
{
	using ObjectClassInfo = Vadon::Core::ObjectClassInfo;
	using ObjectClassInfoList = Vadon::Core::ObjectClassInfoList;

	using ObjectPropertyInfo = Vadon::Core::ObjectPropertyInfo;
	using ObjectPropertyInfoList = Vadon::Core::ObjectPropertyInfoList;

	using ObjectProperty = Vadon::Core::ObjectProperty;
	using ObjectPropertyList = Vadon::Core::ObjectPropertyList;

	class ObjectSystem : public Vadon::Core::ObjectSystem
	{
	public:
		Object* create_object(std::string_view class_id) override;

		ObjectClassInfo get_class_info(std::string_view class_id) const override;
		ObjectClassInfoList get_class_list() const override;
		ObjectClassInfoList get_subclass_hierarchy(std::string_view class_id) const override;

		ObjectPropertyInfoList get_class_properties(std::string_view class_id, bool recursive = true) const override;

		ObjectPropertyList get_object_properties(Object& object, bool recursive = true) const override;
		Variant get_property(Object& object, std::string_view property) const override;
		void set_property(Object& object, std::string_view property, Variant value) override;
	protected:
		struct ObjectClassData : public Vadon::Core::ObjectClassData
		{
			Vadon::Core::EngineCoreInterface& engine_core;

			ObjectClassInfo class_info;

			ObjectClassData* base_data = nullptr;
			std::vector<ObjectClassData*> subclasses; // NOTE: direct descendants only

			ErasedObjectFactoryFunction factory;

			// FIXME: implement more efficient lookups and bookkeeping!
			std::unordered_map<std::string, MemberFunctionData> methods;
			std::unordered_map<std::string, ObjectPropertyInfo> properties;
		
			ObjectClassData(Vadon::Core::EngineCoreInterface& core) : engine_core(core) {}

			bool internal_bind_method(std::string_view name, MemberFunctionData method) override;
			bool add_property(ObjectPropertyInfo property_info) override;

			bool has_method(std::string_view name) const;
			bool has_property(std::string_view name) const;
		};

		struct ObjectPropertyLookup
		{
			const ObjectClassData* class_data = nullptr;
			const ObjectPropertyInfo* property_info = nullptr;

			bool is_valid() const { return (class_data != nullptr) && (property_info != nullptr); }
		};

		ObjectSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();

		Vadon::Core::ObjectClassData* internal_register_object_class(ClassRegistryInfo class_info, ErasedObjectFactoryFunction factory) override;
		void internal_get_class_properties(const ObjectClassData& class_data, ObjectPropertyInfoList& properties) const;

		ObjectPropertyLookup internal_find_property_info(const ObjectClassData& class_data, std::string_view property_name) const;
		const MemberFunctionData* internal_find_method(const ObjectClassData& class_data, std::string_view method_name) const;
		Variant invoke_method(Object& object, std::string_view method_name, const MemberFunctionData& method_data, VariantArgumentList args = {}) const;

		bool is_instance_of(Object& object, std::string_view class_id) const override;

		// FIXME: implement more efficient lookups and bookkeeping!
		std::unordered_map<std::string, ObjectClassData> m_object_classes;

		friend class EngineCore;
	};
}
#endif