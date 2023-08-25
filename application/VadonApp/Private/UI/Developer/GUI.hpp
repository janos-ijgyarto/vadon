#ifndef VADONAPP_PRIVATE_UI_DEVELOPER_GUI_HPP
#define VADONAPP_PRIVATE_UI_DEVELOPER_GUI_HPP
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Render/Frame/Graph.hpp>
namespace VadonApp::Private::UI::Developer
{
	class GUISystem : public VadonApp::UI::Developer::GUISystem
	{
	public:
		using Implementation = std::unique_ptr<GUISystem>;

		virtual bool initialize() = 0;
		virtual void shutdown() = 0;

		static Implementation get_gui(VadonApp::Core::Application& application);
		static Implementation get_dummy_gui(VadonApp::Core::Application& application);
	protected:
		GUISystem(VadonApp::Core::Application& application);

		Vadon::Render::FrameGraphHandle m_frame_graph;
	};
}
#endif