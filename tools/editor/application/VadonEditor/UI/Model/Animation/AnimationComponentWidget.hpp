#ifndef VADONEDITOR_UI_MODEL_ANIMATION_ANIMATIONCOMPONENTWIDGET_HPP
#define VADONEDITOR_UI_MODEL_ANIMATION_ANIMATIONCOMPONENTWIDGET_HPP
#include <VadonEditor/UI/Model/Animation/ui_AnimationComponentWidget.h>
#include <QHash>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class Animation;
	class Component;
	class Entity;
	class Resource;
	class Scene;
}
namespace VadonEditor::UI
{
	struct EntityAnimationChannelEntry
	{
		QUuid animation_id;
		QUuid channel_id;
		QUuid entity_id;
		QUuid component_id;
		QUuid property_id;

		bool load_data(Core::Application& application, const QVariantMap& properties);
		bool store_data(Core::Application& application, QVariantMap& properties) const;
	};

	class AnimationEditor;
	class AnimationComponentWidget : public QWidget
	{
		Q_OBJECT
	public:
		AnimationComponentWidget(Model::Scene* owner_scene, Model::Entity* entity, Model::Component* component, QWidget* parent = nullptr);
	signals:
		void animation_modified();
	private slots:
		void manage_animations_clicked();
		void animation_selected(int index);

		void create_animation_triggered();
		void clear_animation_triggered();

		void new_channel_requested(const QUuid& animation_id);
		void remove_channel_requested(const QUuid& animation_id, const QUuid& channel_id);

		void new_channel_entity_selected(const QUuid& entity_id);
		void new_channel_component_property_selected(const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id);
		void clear_channel_request();

		void animation_added(const QUuid& animation_id);
		void animation_edited(const QUuid& animation_id);
		void animation_removed(const QUuid& animation_id);

		void animation_channel_added(const QUuid& animation_id, const QUuid& channel_id);
		void animation_channel_edited(const QUuid& animation_id, const QUuid& channel_id);
		void animation_channel_removed(const QUuid& animation_id, const QUuid& channel_id);
	private:
		void create_resource();
		void remove_resource();

		void open_animation(const QUuid& animation_id);

		void add_channel_entry(const EntityAnimationChannelEntry& entry);
		void remove_channel_entry(const QUuid& animation_id, const QUuid& channel_id);

		void generate_widgets();
		void clear_widgets();

		void update_animation_list_property_data() const;
		void update_animation_entries_property_data() const;
		void notify_component_modified() const;

		Model::Resource* get_entity_animation_resource() const;
		Model::Resource* find_animation_in_entity_animation(const QUuid& animation_id) const;

		void store_animation_data(const QUuid& animation_id) const;

		Ui::AnimationComponentWidget m_ui;

		Model::Scene* m_owner_scene;
		Model::Entity* m_entity;
		Model::Component* m_component;

		struct AnimationEditorEntry
		{
			Model::Animation* model;
			AnimationEditor* editor;
		};
		
		QHash<QUuid, AnimationEditorEntry> m_animation_editor_lookup;
		QHash<QUuid, QHash<QUuid, EntityAnimationChannelEntry>> m_channel_entry_lookup;
		AnimationEditor* m_focused_editor;

		QUuid m_new_channel_request_id;
	};
}
#endif