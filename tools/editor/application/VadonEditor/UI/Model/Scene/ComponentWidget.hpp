#ifndef VADONEDITOR_UI_MODEL_SCENE_COMPONENTWIDGET_HPP
#define VADONEDITOR_UI_MODEL_SCENE_COMPONENTWIDGET_HPP
#include <VadonEditor/UI/Model/Scene/ui_ComponentWidget.h>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class Entity;
	class Scene;
	class Component;
}
namespace VadonEditor::UI
{
	class PropertyWidget;
	class ComponentWidget : public QWidget
	{
		Q_OBJECT
	public:
		ComponentWidget(Model::Scene* owner_scene, Model::Entity* entity, Model::Component* component, QWidget* parent = nullptr);

		bool initialize();

		const Model::Component* get_component() const { return m_component; }

		QString get_name_label() const { return m_ui.componentNameLabel->text(); }
	signals:
		// NOTE: using pass-by-value to ensure the ID remains valid (since the widget itself will be destroyed)
		void remove_requested(QUuid component_id);
	private slots:
		void internal_property_edited(const QUuid& property_id);
		void remove_clicked();

		void edit_animation_clicked();
	private:
		PropertyWidget* find_property_widget(const QUuid& property_id) const;

		Ui::ComponentWidget m_ui;

		Model::Scene* m_owner_scene;
		Model::Entity* m_entity;
		Model::Component* m_component;
	};
}
#endif