#ifndef VADONEDITOR_CORE_EDITOR_HPP
#define VADONEDITOR_CORE_EDITOR_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <VadonEditor/Core/System/Registry.hpp>
#include <memory>

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
	class Editor : public SystemRegistry
	{
	public:
		VADONEDITOR_API Editor();
		VADONEDITOR_API ~Editor();

		VADONEDITOR_API int execute(int argc, char* argv[]);

		VADONEDITOR_API VadonApp::Core::Application& get_engine_app();
		VADONEDITOR_API Vadon::Core::EngineCoreInterface& get_engine_core();

		float get_delta_time() const;

		// FIXME: implement a proper CLI parser!
		bool has_command_line_arg(std::string_view name) const;
		std::string get_command_line_arg(std::string_view name) const;

		VADONEDITOR_API static void init_editor_environment(Vadon::Core::EngineEnvironment& environment);
	protected:
		virtual bool post_init() { return true; }
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif