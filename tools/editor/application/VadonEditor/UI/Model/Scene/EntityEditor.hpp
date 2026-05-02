#ifndef VADONEDITOR_UI_MODEL_SCENE_ENTITYEDITOR_HPP
#define VADONEDITOR_UI_MODEL_SCENE_ENTITYEDITOR_HPP
#include <VadonEditor/UI/Model/Scene/ui_EntityEditor.h>
namespace VadonEditor::Model
{
	class Entity;
	struct Component;
}
namespace VadonEditor::UI
{
	class EntityEditor : public QWidget
	{
		Q_OBJECT
	public:
	signals:
		void component_added(const QUuid& entity_id, const QUuid& component_id);
		void component_removed(const QUuid& entity_id, const QUuid& component_id);
		void component_property_edited(const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id);

		void entity_name_changed(const QUuid& entity_id, const QString& text);
	private slots:
		void internal_name_changed(const QString& text);
		void add_component_clicked();
		void new_component_selected(const QUuid& component_type);

		void internal_component_property_edited(const QUuid& component_id, const QUuid& property_id);
		void component_remove_requested(const QUuid& component_id);
	private:
		EntityEditor(Model::Entity* entity, QWidget* parent = nullptr, Qt::WindowType type = Qt::WindowType::Widget);

		bool initialize();
		bool update_title();

		bool internal_add_component_widget(Model::Component* component);

		Ui::EntityEditor m_ui;
		Model::Entity* m_entity;

		friend class SceneTree;
	};
}
#endif