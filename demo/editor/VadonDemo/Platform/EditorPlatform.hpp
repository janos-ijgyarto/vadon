#ifndef VADONDEMO_PLATFORM_EDITORPLATFORM_HPP
#define VADONDEMO_PLATFORM_EDITORPLATFORM_HPP
#include <VadonApp/Platform/Input/Input.hpp>
#include <Vadon/Utilities/Enum/EnumClass.hpp>
#include <array>
namespace VadonDemo::Core
{
    class Editor;
}
namespace VadonDemo::Platform
{
    enum class EditorInputAction
    {
        CAMERA_UP,
        CAMERA_DOWN,
        CAMERA_LEFT,
        CAMERA_RIGHT,
        CAMERA_ZOOM,
        ACTION_COUNT
    };

	class EditorPlatform
	{
    public:
        VadonApp::Platform::InputActionHandle get_action(EditorInputAction action) const { return m_input_actions[Vadon::Utilities::to_integral(action)]; }
	private:
        EditorPlatform(Core::Editor& editor);
		bool initialize();

        Core::Editor& m_editor;

        std::array<VadonApp::Platform::InputActionHandle, Vadon::Utilities::to_integral(EditorInputAction::ACTION_COUNT)> m_input_actions;

        friend Core::Editor;
	};
}
#endif