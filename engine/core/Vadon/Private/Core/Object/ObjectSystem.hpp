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

	using ObjectProperty = Vadon::Core::ObjectProperty;
	using ObjectPropertyList = Vadon::Core::ObjectPropertyList;

	class ObjectSystem : public Vadon::Core::ObjectSystem
	{
	public:
		Object* create_object(std::string_view class_id) override;

		ObjectClassInfo get_class_info(std::string_view class_id) const override;
		ObjectClassInfoList get_class_list() const override;
		ObjectClassInfoList get_subclass_hierarchy(std::string_view class_id) const override;

		ObjectPropertyList get_class_properties(std::string_view class_id, bool recursive = true) const override;

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
			std::unordered_map<std::string, MemberFunctionData> methods;
			std::unordered_map<std::string, ObjectProperty> properties;
		
			ObjectClassData(Vadon::Core::EngineCoreInterface& core) : engine_core(core) {}

			bool internal_bind_method(std::string_view name, MemberFunctionData method) override;
			bool add_property(ObjectProperty property) override;

			bool has_method(std::string_view name) const;
			bool has_property(std::string_view name) const;

			Variant invoke_method(Object& object, std::string_view name, VariantArgumentList args = {}) const;
		};

		ObjectSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();

		Vadon::Core::ObjectClassData* internal_register_object_class(ClassRegistryInfo class_info, ErasedObjectFactoryFunction factory) override;
		void internal_get_class_properties(const ObjectClassData& class_data, ObjectPropertyList& properties) const;

		// FIXME: implement more efficient lookups and bookkeeping!
		std::unordered_map<std::string, ObjectClassData> m_object_classes;

		friend class EngineCore;
	};
}
#endif