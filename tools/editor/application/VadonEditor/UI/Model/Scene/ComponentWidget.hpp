#ifndef VADONEDITOR_UI_MODEL_SCENE_COMPONENTWIDGET_HPP
#define VADONEDITOR_UI_MODEL_SCENE_COMPONENTWIDGET_HPP
#include <VadonEditor/UI/Model/Scene/ui_ComponentWidget.h>
namespace VadonEditor::UI
{
	class ComponentWidget : public QWidget
	{
		Q_OBJECT
	public:
		ComponentWidget(QWidget* parent = nullptr);
	private:
		Ui::ComponentWidget m_ui;
	};
}
#endif