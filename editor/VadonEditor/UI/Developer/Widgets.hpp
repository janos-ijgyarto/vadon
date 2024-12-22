#ifndef VADONEDITOR_UI_DEVELOPER_WIDGETS_HPP
#define VADONEDITOR_UI_DEVELOPER_WIDGETS_HPP
#include <VadonEditor/UI/Developer/GUI.hpp>
#include <VadonApp/UI/Developer/Widgets/Dialog.hpp>
#include <VadonApp/UI/Developer/Widgets/FileBrowser.hpp>
namespace VadonEditor::UI::Developer
{
	using Dialog = VadonApp::UI::Developer::Dialog;

	using MessageDialog = VadonApp::UI::Developer::MessageDialog;
	using ConfirmDialog = VadonApp::UI::Developer::ConfirmDialog;

	using FileBrowserWidget = VadonApp::UI::Developer::FileBrowserWidget;
	using FileBrowserDialog = VadonApp::UI::Developer::FileBrowserDialog;
}
#endif