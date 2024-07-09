#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Core/Environment.hpp>

#include <Vadon/ECS/Component/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>

#include <iostream>

namespace Vadon::Core
{
	EngineEnvironment* EngineEnvironment::s_instance = nullptr;

	EngineEnvironment::EngineEnvironment()
		: component_registry(std::make_unique<Vadon::ECS::ComponentRegistry>())
		, type_registry(std::make_unique<Vadon::Utilities::TypeRegistry>())
	{
	}

	EngineEnvironment::~EngineEnvironment() = default;

	void EngineEnvironment::initialize(EngineEnvironment& instance)
	{
		if ((s_instance != nullptr) && (s_instance != &instance))
		{
			std::cerr << "Engine environment error: attempted to set different environment instances!\n";
			assert(false);
			std::terminate();
		}

		s_instance = &instance;
	}
}