#include <VadonEditor/UI/Model/Scene/SceneTree.hpp>

#include <VadonEditor/Model/Resource/Resource.hpp>
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

	void SceneTree::entity_double_clicked(const QModelIndex& index)
	{
		emit(entity_opened(m_scene->get_resource()->get_info().id, index));
	}
}