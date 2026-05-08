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
	class PropertyWidget;
	class ComponentWidget : public QWidget
	{
		Q_OBJECT
	public:
		ComponentWidget(Model::Entity* entity, Model::Component* component, QWidget* parent = nullptr);

		bool initialize();

		const Model::Component* get_component() const { return m_component; }
	signals:
		void property_edited(const QUuid& component_id, const QUuid& property_id);
		void remove_requested(const QUuid& component_id);
	private slots:
		void internal_property_edited(const QUuid& property_id);
		void remove_clicked();
	private:
		PropertyWidget* find_property_widget(const QUuid& property_id) const;

		Ui::ComponentWidget m_ui;

		Model::Entity* m_entity;
		Model::Component* m_component;
	};
}
#endif