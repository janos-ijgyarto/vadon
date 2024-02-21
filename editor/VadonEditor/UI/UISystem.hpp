#ifndef VADONEDITOR_UI_UISYSTEM_HPP
#define VADONEDITOR_UI_UISYSTEM_HPP
#include <VadonEditor/UI/Module.hpp>
#include <memory>
#include <functional>
namespace VadonEditor::UI
{
	using ConsoleCallback = std::function<void()>;
	using UICallback = std::function<void(Core::Editor&)>;

	class UISystem : public UISystemBase<UISystem>
	{
	public:
		~UISystem();

		void register_console_command(std::string_view command, const ConsoleCallback& callback);
		void register_ui_element(const UICallback& callback);
	private:
		UISystem(Core::Editor& editor);

		bool initialize();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Editor;
	};
}
#endif