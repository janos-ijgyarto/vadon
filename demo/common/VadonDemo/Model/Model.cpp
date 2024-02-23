#include <VadonDemo/Model/Model.hpp>

#include <VadonDemo/Model/Node.hpp>

#include <Vadon/Core/CoreInterface.hpp>
#include <Vadon/Core/Object/ObjectSystem.hpp>

namespace VadonDemo::Model
{
	Model::Model(Vadon::Core::EngineCoreInterface& engine_core)
		: m_engine_core(engine_core)
	{
	}

	bool Model::initialize()
	{
		Vadon::Core::ObjectSystem& obj_system = m_engine_core.get_system<Vadon::Core::ObjectSystem>();

		obj_system.register_object_class<Node2D>();
		obj_system.register_object_class<Orbiter>();
		obj_system.register_object_class<Pivot>();

		return true;
	}
}