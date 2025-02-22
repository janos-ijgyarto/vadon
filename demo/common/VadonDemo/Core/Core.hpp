#ifndef VADONDEMO_CORE_CORE_HPP
#define VADONDEMO_CORE_CORE_HPP
#include <VadonDemo/VadonDemoCommon.hpp>

#include <VadonDemo/Model/Model.hpp>
#include <VadonDemo/Render/Render.hpp>
#include <VadonDemo/UI/UI.hpp>
#include <VadonDemo/View/View.hpp>
namespace Vadon::Core
{
	class EngineEnvironment;
	class EngineCoreInterface;
}
namespace VadonDemo::Core
{
	// TODO: have this class contain the other systems?
	class Core
	{
	public:
		VADONDEMO_API Core(Vadon::Core::EngineCoreInterface& engine_core);		
		VADONDEMO_API static void init_environment(Vadon::Core::EngineEnvironment& environment);

		VADONDEMO_API bool initialize();

		Vadon::Core::EngineCoreInterface& get_engine_core() { return m_engine_core; }

		Model::Model& get_model() { return m_model; }
		Render::Render& get_render() { return m_render; }
		UI::UI& get_ui() { return m_ui; }
		View::View& get_view() { return m_view; }
	private:
		Vadon::Core::EngineCoreInterface& m_engine_core;

		Model::Model m_model;
		Render::Render m_render;
		UI::UI m_ui;
		View::View m_view;
	};
}
#endif