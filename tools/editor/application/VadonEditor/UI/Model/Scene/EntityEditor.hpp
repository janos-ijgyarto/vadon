#ifndef VADONEDITOR_UI_MODEL_SCENE_ENTITYEDITOR_HPP
#define VADONEDITOR_UI_MODEL_SCENE_ENTITYEDITOR_HPP
#include <VadonEditor/UI/Model/Scene/ui_EntityEditor.h>
namespace VadonEditor::UI
{
	class EntityEditor : public QWidget
	{
		Q_OBJECT
	public:
	private:
		EntityEditor(QWidget* parent = nullptr);

		Ui::EntityEditor m_ui;

		friend class SceneManager;
	};
}
#endif