#ifndef VADONDEMO_PLATFORM_EDITORPLATFORM_HPP
#define VADONDEMO_PLATFORM_EDITORPLATFORM_HPP
#include <Vadon/Math/Vector.hpp>
#include <Vadon/Utilities/Enum/EnumClass.hpp>
namespace VadonDemo::Core
{
    class Editor;
}
namespace VadonDemo::Platform
{
    struct EditorWindow
    {
        Vadon::Math::Vector2i size;
        uintptr_t platform_handle;
    };

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
        // FIXME: rework Application to be a set of "engine extensions" we can use,
        // granting access to Input Action logic while delegating Application management
        // to the editor app
        bool is_action_pressed(EditorInputAction action) const { return m_actions_pressed[static_cast<size_t>(action)]; }
        float get_action_strength(EditorInputAction action) const { return m_action_strengths[static_cast<size_t>(action)]; }

        const EditorWindow& get_window() const { return m_window; }
	private:
        EditorPlatform(Core::Editor& editor);
		bool initialize();
        void update();

        void editor_connected();

        void process_message(const char* data, size_t size);

        void request_main_viewport();

        void set_action_pressed(EditorInputAction action, bool pressed) { m_actions_pressed[static_cast<size_t>(action)] = pressed; }
        void set_action_strength(EditorInputAction action, float strength) { m_action_strengths[static_cast<size_t>(action)] = strength; }

        void clear_actions()
        {
            for (size_t action_index = 0; action_index < static_cast<size_t>(EditorInputAction::ACTION_COUNT); ++action_index)
            {
                m_actions_pressed[action_index] = false;
                m_action_strengths[action_index] = 0.0f;
            }
        }

        Core::Editor& m_editor;
        EditorWindow m_window;

        bool m_actions_pressed[static_cast<size_t>(EditorInputAction::ACTION_COUNT)];
        float m_action_strengths[static_cast<size_t>(EditorInputAction::ACTION_COUNT)];

        friend Core::Editor;
	};
}
#endif