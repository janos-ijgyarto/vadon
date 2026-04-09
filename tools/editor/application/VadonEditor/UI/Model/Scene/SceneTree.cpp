#include <VadonEditor/UI/Model/Scene/SceneTree.hpp>

namespace VadonEditor::UI
{
	SceneTree::SceneTree(QWidget* parent)
		: QWidget(parent)
		, m_application(nullptr)
	{
		m_ui.setupUi(this);
	}

	bool SceneTree::initialize(Core::Application& application)
	{
		m_application = &application;
		return true;
	}
}