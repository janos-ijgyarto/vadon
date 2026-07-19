#ifndef VADONEDITOR_UI_MODEL_ANIMATION_ANIMATIONEDITOR_HPP
#define VADONEDITOR_UI_MODEL_ANIMATION_ANIMATIONEDITOR_HPP
#include <VadonEditor/UI/Model/Animation/ui_AnimationEditor.h>
#include <QGraphicsView>
#include <QTimer>
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
        int track_spacing = 2;
        int default_track_height = c_default_channel_height;
    };

    struct AnimationEditorTheme
    {
        AnimationEditorConstants constants;
        AnimationEditorColorTheme colors;
    };

    struct AnimationKeyData
    {
        QVariant value;
        qreal start_time;
        qreal duration;
    };

    class AnimationChannelData
    {
    public:
        AnimationChannelData(const QString& name, int height = AnimationEditorConstants::c_default_channel_height)
            : m_name(name)
            , m_height(height)
        {

        }

        void add_key(const AnimationKeyData& key_data)
        {
            m_keys.push_back(key_data);
        }

        const QList<AnimationKeyData>& get_keys() const { return m_keys; }
        QList<AnimationKeyData>& get_keys() { return m_keys; }

        const QString& get_name() const { return m_name; }
        int get_height() const { return m_height; }
    private:
        QString m_name;
        int m_height;
        QList<AnimationKeyData> m_keys;
    };

	class AnimationEditor : public QWidget
	{
		Q_OBJECT
	public:
        AnimationEditor(QWidget* parent = nullptr);
        ~AnimationEditor();

        const AnimationEditorTheme& get_theme() const { return m_theme; }
        const QList<AnimationChannelData>& get_channel_data() const { return m_channel_data; }

        qreal get_horizontal_zoom() const { return m_horizontal_zoom; }
        qreal get_vertical_zoom() const { return m_vertical_zoom; }

        qreal calculate_scene_height() const;
    private slots:
        void playback_start_triggered();
        void playback_stop_triggered();

        void add_channel_triggered();

        void timeline_zoom_changed();
	private:
        void initialize();

        void transform_updated();
        void time_updated();

        void playback_timer_update();
        void reset_playback();

		Ui::AnimationEditor m_ui;

        AnimationEditorTheme m_theme;
        QList<AnimationChannelData> m_channel_data;

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