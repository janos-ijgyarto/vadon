#ifndef VADONEDITOR_UI_MODEL_ANIMATION_ANIMATIONEDITOR_HPP
#define VADONEDITOR_UI_MODEL_ANIMATION_ANIMATIONEDITOR_HPP
#include <VadonEditor/UI/Model/Animation/ui_AnimationEditor.h>
#include <VadonEditor/Model/Animation/Animation.hpp>
#include <QGraphicsView>
#include <QTimer>
namespace VadonEditor::Model
{
    class Resource;
}
namespace VadonEditor::UI
{
    // Implementation partially based on https://github.com/hasielhassan/QtEditorialTimelineWidget
    struct AnimationEditorColorTheme
    {
        QColor time_label_background;
        QColor time_label_text;
        QColor ruler_background;
        QColor ruler_tick_major;
        QColor ruler_tick_minor;
        QColor playhead_color;
        QColor channel_header_background;
        QColor channel_header_text;
        QColor channel_lane_background_1;
        QColor channel_lane_background_2;
        QColor channel_lane_border;
        QColor key_fill;
        QColor key_fill_selected;
        QColor key_border;
        QColor end_line_color;
        QColor background_color;
    };

    struct AnimationEditorConstants
    {
        static constexpr int c_default_channel_height = 60;

        int left_margin = 150;
        int top_margin = 30;
        int bottom_margin = 20;
        int channel_spacing = 2;
    };

    struct AnimationEditorTheme
    {
        AnimationEditorConstants constants;
        AnimationEditorColorTheme colors;
    };

	class AnimationEditor : public QWidget
	{
		Q_OBJECT
	public:
        AnimationEditor(Model::Resource* resource, QWidget* parent = nullptr);
        ~AnimationEditor();

        bool initialize();

        const Model::Animation& get_animation() const { return m_animation; }
        const AnimationEditorTheme& get_theme() const { return m_theme; }
        QGraphicsView* get_view() const { return m_ui.timelineView; }

        qreal get_horizontal_zoom() const { return m_horizontal_zoom; }
        qreal get_vertical_zoom() const { return m_vertical_zoom; }

        qreal get_timeline_width() const;
        qreal calculate_scene_height() const;

        void channel_selected(const QUuid& channel_id);
        void key_moved(const QUuid& channel_id, int key_id);

        void channel_context_menu_requested(const QUuid& channel_id, const QPoint& screen_pos, const QPointF& item_pos);
    private slots:
        void playback_start_triggered();
        void playback_stop_triggered();

        void add_channel_triggered();
        void remove_channel_triggered();

        void add_key_triggered();
        void remove_keys_triggered();

        void timeline_zoom_changed();

        void save_triggered();

        void animation_edited();

        void animation_name_changed(const QString& text);
        void channel_tag_text_changed(const QString& text);
        void key_time_changed(double value);
        void key_value_changed();

        void item_selection_changed();
	private:
        void update_title();

        void transform_updated();
        void time_updated();

        void playback_timer_update();
        void reset_playback();

        Model::Resource* m_resource;
        Model::Animation m_animation;

		Ui::AnimationEditor m_ui;

        AnimationEditorTheme m_theme;

        QGraphicsScene m_graphics_scene;

        struct InternalGraphics;
        std::unique_ptr<InternalGraphics> m_internal_graphics;

        QTimer m_timer;

        qreal m_time;
        qreal m_horizontal_zoom;
        qreal m_vertical_zoom;
	};
}
#endif