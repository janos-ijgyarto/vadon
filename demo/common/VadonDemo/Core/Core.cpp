#include <VadonDemo/Core/Core.hpp>

#include <VadonDemo/Core/Component.hpp>

#include <VadonDemo/Model/Model.hpp>
#include <VadonDemo/View/View.hpp>
#include <VadonDemo/UI/UI.hpp>

#include <Vadon/Core/Environment.hpp>

namespace VadonDemo::Core
{
	Core::Core(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);
	}
	
	void Core::register_types()
	{
		CoreComponent::register_component();

		Model::Model::register_types();
		View::View::register_types();
		UI::UI::register_types();
	}
}