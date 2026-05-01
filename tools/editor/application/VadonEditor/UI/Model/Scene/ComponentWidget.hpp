#ifndef VADONEDITOR_UI_MODEL_SCENE_COMPONENTWIDGET_HPP
#define VADONEDITOR_UI_MODEL_SCENE_COMPONENTWIDGET_HPP
#include <VadonEditor/UI/Model/Scene/ui_ComponentWidget.h>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class Entity;
	struct Component;
}
namespace VadonEditor::UI
{
	class ComponentWidget : public QWidget
	{
		Q_OBJECT
	public:
		ComponentWidget(Model::Entity* entity, Model::Component* component, QWidget* parent = nullptr);

		bool initialize();
	signals:
		void property_edited(const QUuid& property_id);
	private:
		Ui::ComponentWidget m_ui;

		Model::Entity* m_entity;
		Model::Component* m_component;
	};
}
#endif