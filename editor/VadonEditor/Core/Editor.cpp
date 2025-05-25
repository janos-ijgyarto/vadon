#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Platform/PlatformInterface.hpp>
#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/View/ViewSystem.hpp>

#include <VadonApp/Core/Application.hpp>

#include <Vadon/Core/Environment.hpp>

namespace VadonEditor::Core
{
	struct Editor::Internal
	{
		Vadon::Core::EngineCoreInterface& m_engine_core;
		Editor& m_editor;

		ProjectManager m_project_manager;

		Model::ModelSystem m_model_system;
		Platform::PlatformInterface m_platform_interface;
		UI::UISystem m_ui_system;
		View::ViewSystem m_view_system;

		VadonApp::Core::Application::Instance m_engine_app;

		Internal(Editor& editor, Vadon::Core::EngineCoreInterface& engine_core)
			: m_engine_core(engine_core)
			, m_editor(editor)
			, m_project_manager(editor)
			, m_model_system(editor)
			, m_platform_interface(editor)
			, m_ui_system(editor)
			, m_view_system(editor)
		{
			m_engine_app = VadonApp::Core::Application::create_instance(engine_core);
		}

		bool initialize(const Configuration& editor_config)
		{
			if (init_application(editor_config.app_config) == false)
			{
				Vadon::Core::Logger::log_message("Failed to initialize engine application!\n");
				return false;
			}

			if (m_project_manager.initialize() == false)
			{
				return false;
			}

			if (m_platform_interface.initialize() == false)
			{
				return false;
			}

			if (m_ui_system.initialize() == false)
			{
				return false;
			}

			if (m_view_system.initialize() == false)
			{
				return false;
			}

			if (m_model_system.initialize() == false)
			{
				return false;
			}

			Vadon::Core::Logger::log_message("Vadon Editor initialized.\n");
			return true;
		}

		bool init_application(const VadonApp::Core::Configuration& app_config)
		{
			return m_engine_app->initialize(app_config);
		}

		void shutdown()
		{
			m_engine_app->shutdown();

			Vadon::Core::Logger::log_message("Vadon Editor successfully shut down.\n");
		}
	};

	Editor::Editor(Vadon::Core::EngineEnvironment& environment, Vadon::Core::EngineCoreInterface& engine_core)
		: m_internal(std::make_unique<Internal>(*this, engine_core))
	{
		VadonApp::Core::Application::init_application_environment(environment);
		Vadon::Core::EngineEnvironment::initialize(environment);
	}

	Editor::~Editor() = default;

	VadonApp::Core::Application& Editor::get_engine_app()
	{
		return *m_internal->m_engine_app;
	}

	Vadon::Core::EngineCoreInterface& Editor::get_engine_core()
	{
		return get_engine_app().get_engine_core();
	}
	
	bool Editor::initialize(const Configuration& editor_config)
	{
		return m_internal->initialize(editor_config);
	}

	void Editor::shutdown()
	{
		m_internal->shutdown();
	}
}