#ifndef VADON_CORE_OBJECT_OBJECT_HPP
#define VADON_CORE_OBJECT_OBJECT_HPP
#include <Vadon/Common.hpp>
#include <Vadon/Utilities/TypeInfo/TypeName.hpp>
namespace Vadon::Core
{
	class EngineCoreInterface;
	class ObjectClassData;

	// Based on Godot's Object API
	// FIXME: implement handles to use as object ID, store per-instance metadata in system (could speed up RTTI?)
	class Object
	{
	public:
		using _Base = Object;
		using _Class = Object;

		VADONCOMMON_API Object(EngineCoreInterface& engine_core);

		static constexpr std::string_view static_class_id() { return "Vadon::Core::Object"; }
		virtual std::string_view get_class_id() const { return static_class_id(); }

		VADONCOMMON_API static void initialize_class(ObjectClassData& class_data);
	protected:
		VADONCOMMON_API static void bind_methods(ObjectClassData& class_data);
		inline static void (*_get_bind_methods())(ObjectClassData&) { return &bind_methods; }

		EngineCoreInterface& m_engine_core;
	};
}

#define VADON_OBJECT_CLASS(_class, _base) \
public: \
	using _Class = _class; \
	using _Base = _base; \
	static constexpr std::string_view static_class_id() { return Vadon::Utilities::TypeName<_Class>::trimmed(); } \
	std::string_view get_class_id() const override { return static_class_id(); } \
protected: \
	inline static void (*_get_bind_methods())(Vadon::Core::ObjectClassData&) \
	{ \
		return &_class::bind_methods; \
	} \
public: \
	static void initialize_class(Vadon::Core::ObjectClassData& class_data) \
	{ \
		if (_Class::_get_bind_methods() != _Base::_get_bind_methods()) \
		{ \
			bind_methods(class_data); \
		} \
	}
#endif