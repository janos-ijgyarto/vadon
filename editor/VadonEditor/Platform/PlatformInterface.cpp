#include <VadonEditor/Platform/PlatformInterface.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>

namespace
{
	constexpr int c_default_window_width = 1280;
	constexpr int c_default_window_height = 1024;
}

namespace VadonEditor::Platform
{
	struct PlatformInterface::Internal
	{
		VadonApp::Platform::WindowHandle m_main_window;

		bool initialize(Core::Editor& editor)
		{
			// FIXME: have client provide platform config
			VadonApp::Platform::WindowInfo main_window_info;
			main_window_info.title = "Vadon Editor"; // TODO: version numbering?
			main_window_info.position = Vadon::Utilities::Vector2i(-1, -1);

			// FIXME: add casting to CLI parsing so we don't need to do it here
			// TODO: make these args global vars that are possible to toggle at all times (similar to Unreal)?
			const std::string window_width_str = editor.get_command_line_arg("window_w");
			const std::string window_height_str = editor.get_command_line_arg("window_h");

			main_window_info.size = Vadon::Utilities::Vector2i(window_width_str.empty() ? c_default_window_width : std::stoi(window_width_str),
			window_height_str.empty() ? c_default_window_height : std::stoi(window_height_str));

			main_window_info.flags = VadonApp::Platform::WindowFlags::SHOWN | VadonApp::Platform::WindowFlags::RESIZABLE;

			VadonApp::Platform::PlatformInterface& platform_interface = editor.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
			m_main_window = platform_interface.create_window(main_window_info);

			if (m_main_window.is_valid() == false)
			{
				return false;
			}

			return true;
		}

		void update(Core::Editor& editor)
		{
			VadonApp::Platform::PlatformInterface& platform_interface = editor.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
			platform_interface.poll_events();
		}
	};

	PlatformInterface::~PlatformInterface() = default;

	VadonApp::Platform::WindowHandle PlatformInterface::get_main_window() const
	{
		return m_internal->m_main_window;
	}

	PlatformInterface::PlatformInterface(Core::Editor& editor)
		: System(editor)
		, m_internal(std::make_unique<Internal>())
	{

	}

	bool PlatformInterface::initialize()
	{
		return m_internal->initialize(m_editor);
	}

	void PlatformInterface::update()
	{
		m_internal->update(m_editor);
	}
}