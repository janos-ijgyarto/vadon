#ifndef VADONAPP_UI_DEVELOPER_CONFIGURATION_HPP
#define VADONAPP_UI_DEVELOPER_CONFIGURATION_HPP
namespace VadonApp::UI::Developer
{
	struct GUIConfiguration
	{
		int32_t frame_count = 0; // Number of frames to cache (helps client code with multithreaded contexts)
	};
}
#endif