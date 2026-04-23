#include <VadonEditor/UI/Model/Scene/SceneTree.hpp>

#include <VadonEditor/Model/Scene/Scene.hpp>

namespace VadonEditor::UI
{
	SceneTree::SceneTree(Model::Scene* scene, QWidget* parent)
		: QWidget(parent)
		, m_scene(scene)
	{
		m_ui.setupUi(this);

		m_ui.treeView->setModel(&scene->get_entity_model().get_qt_model());
	}
}