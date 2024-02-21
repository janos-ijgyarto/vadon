#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Core/Object/Object.hpp>

#include <Vadon/Core/Object/ClassData.hpp>

namespace Vadon::Core
{
	Object::Object(EngineCoreInterface& engine_core)
		: m_engine_core(engine_core)
	{
	}

	void Object::bind_methods(ObjectClassData& /*class_data*/)
	{
		// TODO!!!
	}

	void Object::initialize_class(ObjectClassData& class_data)
	{
		bind_methods(class_data);
	}
}