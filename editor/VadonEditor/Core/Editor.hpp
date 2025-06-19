#ifndef VADONEDITOR_CORE_EDITOR_HPP
#define VADONEDITOR_CORE_EDITOR_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <VadonEditor/Core/System/Registry.hpp>

#include <VadonApp/Core/Configuration.hpp>

namespace VadonApp::Core
{
	class Application;
}

namespace Vadon::Core
{
	class EngineCoreInterface;
	class EngineEnvironment;
}

namespace VadonEditor::Core
{
	struct Configuration
	{
		VadonApp::Core::Configuration app_config;
		// TODO: editor settings?
	};

	class Editor : public SystemRegistry
	{
	public:
		VADONEDITOR_API Editor(Vadon::Core::EngineEnvironment& environment, Vadon::Core::EngineCoreInterface& engine_core);
		VADONEDITOR_API ~Editor();

		VADONEDITOR_API bool initialize(const Configuration& editor_config);
		VADONEDITOR_API void shutdown();

		VADONEDITOR_API VadonApp::Core::Application& get_engine_app();
		VADONEDITOR_API Vadon::Core::EngineCoreInterface& get_engine_core();
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif