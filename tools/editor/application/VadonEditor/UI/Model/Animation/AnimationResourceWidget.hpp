#ifndef VADONEDITOR_UI_MODEL_ANIMATION_ANIMATIONRESOURCEWIDGET_HPP
#define VADONEDITOR_UI_MODEL_ANIMATION_ANIMATIONRESOURCEWIDGET_HPP
#include <VadonEditor/UI/Model/Animation/ui_AnimationResourceWidget.h>
namespace VadonEditor::Model
{
	class Animation;
	class Resource;
}
namespace VadonEditor::UI
{
	class AnimationEditor;
	class AnimationResourceWidget : public QWidget
	{
		Q_OBJECT
	public:
		AnimationResourceWidget(Model::Resource* resource, QWidget* parent = nullptr);
		~AnimationResourceWidget();
	private slots:
		void channel_added(const QUuid& animation_id, const QUuid& channel_id);
		void channel_edited(const QUuid& animation_id, const QUuid& channel_id);
		void channel_removed(const QUuid& animation_id, const QUuid& channel_id);

		void save_triggered();
	private:
		void update_title();
		QString get_label() const;

		Ui::AnimationResourceWidget m_ui;

		Model::Resource* m_resource;
		Model::Animation* m_animation_model;
		AnimationEditor* m_animation_editor;
	};
}
#endif