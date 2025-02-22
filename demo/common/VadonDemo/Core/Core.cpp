#include <VadonDemo/Core/Core.hpp>

#include <VadonDemo/Core/Component.hpp>

#include <Vadon/Core/Environment.hpp>

namespace VadonDemo::Core
{
	Core::Core(Vadon::Core::EngineCoreInterface& engine_core)
		: m_engine_core(engine_core)
		, m_model(engine_core)
		, m_render(*this)
		, m_ui(*this)
		, m_view(*this)
	{
	}
	
	void Core::init_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);
	}

	bool Core::initialize()
	{
		CoreComponent::register_component();

		Model::Model::register_types();
		Render::Render::register_types();
		UI::UI::register_types();
		View::View::register_types();

		if (m_render.initialize() == false)
		{
			return false;
		}

		if (m_model.initialize() == false)
		{
			return false;
		}

		if (m_view.initialize() == false)
		{
			return false;
		}

		if (m_ui.initialize() == false)
		{
			return false;
		}

		return true;
	}
}