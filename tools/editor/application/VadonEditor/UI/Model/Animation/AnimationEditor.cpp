#include <VadonEditor/UI/Model/Animation/AnimationEditor.hpp>

#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QSlider>
namespace
{
    constexpr VadonEditor::UI::AnimationEditorConstants c_default_animation_editor_constants;
    constexpr int c_animation_editor_scene_width = 2000;

    const VadonEditor::UI::AnimationEditorColorTheme c_animation_editor_dark_theme = VadonEditor::UI::AnimationEditorColorTheme{
        .time_label_background = QColor::fromString("#141414"),
        .time_label_text = QColor::fromString("#FFFFFF"),
        .ruler_background = QColor::fromString("#1E1E1E"),
        .ruler_tick_major = QColor::fromString("#FFFFFF"),
        .ruler_tick_minor = QColor::fromString("#808080"),
        .playhead_color = QColor::fromString("#FFA500"),
        .channel_header_background = QColor::fromString("#282828"),
        .channel_header_text = QColor::fromString("#FFFFFF"),
        .channel_lane_background_1 = QColor::fromString("#323232"),
        .channel_lane_background_2 = QColor::fromString("#3E3E3E"),
        .channel_lane_border = QColor::fromString("#505050"),
        .key_fill = QColor::fromString("#6496C8"),
        .key_fill_selected = QColor::fromString("#96C8FF"),
        .key_border = QColor::fromString("#000000"),
        .end_line_color = QColor::fromString("#C83232"),
        .background_color = QColor::fromString("#111111")
    };

    const VadonEditor::UI::AnimationEditorColorTheme c_animation_editor_light_theme = VadonEditor::UI::AnimationEditorColorTheme{
        .time_label_background = QColor::fromString("#F0F0F0"),
        .time_label_text = QColor::fromString("#000000"),
        .ruler_background = QColor::fromString("#E0E0E0"),
        .ruler_tick_major = QColor::fromString("#000000"),
        .ruler_tick_minor = QColor::fromString("#808080"),
        .playhead_color = QColor::fromString("#FF8C00"),
        .channel_header_background = QColor::fromString("#D0D0D0"),
        .channel_header_text = QColor::fromString("#000000"),
        .channel_lane_background_1 = QColor::fromString("#E8E8E8"),
        .channel_lane_background_2 = QColor::fromString("#F0F0F0"),
        .channel_lane_border = QColor::fromString("#A0A0A0"),
        .key_fill = QColor::fromString("#90CAF9"),
        .key_fill_selected = QColor::fromString("#64B5F6"),
        .key_border = QColor::fromString("#000000"),
        .end_line_color = QColor::fromString("#E53935"),
        .background_color = QColor::fromString("#FFFFFF")
    };

    QString frames_to_timecode(qreal frames, int fps = 24)
    {
        const int frames_int = qRound(frames);
        const int seconds = frames_int / fps;
        const int frames_rem = frames_int % fps;
        const int hours = seconds / 3600;
        const int minutes = (seconds % 3600) / 60;
        const int seconds_rem = seconds % 60;

        return QString("%1:%2:%3:%4")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds_rem, 2, 10, QLatin1Char('0'))
            .arg(frames_rem, 2, 10, QLatin1Char('0'));
    }

    class AnimationTimelineRulerItem : public QGraphicsItem
    {
    public:
        AnimationTimelineRulerItem(const VadonEditor::UI::AnimationEditor& anim_editor, QGraphicsItem* parent = nullptr)
            : QGraphicsItem(parent)
            , m_anim_editor(anim_editor)
        {
        }

        QRectF boundingRect() const override
        {
            const int width = (c_animation_editor_scene_width - m_anim_editor.get_theme().constants.left_margin) * m_anim_editor.get_horizontal_zoom();
            return QRectF(0, 0, width, m_anim_editor.get_theme().constants.top_margin);
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
        {
            Q_UNUSED(option);
            Q_UNUSED(widget);
            const QRectF rect = boundingRect();

            painter->fillRect(rect, m_anim_editor.get_theme().colors.ruler_background);

            // TODO: find a way to adjust how many ticks are drawn based on zoom!
            constexpr int c_tick_count = 100;
            const qreal tick_interval = (rect.width() / c_tick_count);

            qreal x_offset = tick_interval;
            draw_major_tick(painter, rect, 0, 0.0);
            for (int tick_index = 1; tick_index < c_tick_count; ++tick_index)
            {
                if (tick_index % 10 == 0)
                {
                    draw_major_tick(painter, rect, x_offset, qreal(tick_index) / qreal(c_tick_count));
                }
                else
                {
                    draw_minor_tick(painter, rect, x_offset);
                }
                x_offset += tick_interval;
            }
            draw_major_tick(painter, rect, x_offset, 1.0);
        }

        void set_horizontal_zoom() {}
    private:
        void draw_major_tick(QPainter* painter, const QRectF& rect, qreal x_offset, qreal value)
        {
            painter->setPen(QPen(m_anim_editor.get_theme().colors.ruler_tick_major));
            painter->drawLine(x_offset, rect.bottom(), x_offset, rect.bottom() - 15);
            painter->drawText(x_offset + 2, rect.bottom() - 17, QString::number(value));
        }

        void draw_minor_tick(QPainter* painter, const QRectF& rect, qreal x_offset)
        {
            painter->setPen(QPen(m_anim_editor.get_theme().colors.ruler_tick_minor));
            painter->drawLine(x_offset, rect.bottom(), x_offset, rect.bottom() - 5);
        }

        const VadonEditor::UI::AnimationEditor& m_anim_editor;
    };

    class AnimationTimelinePlayheadItem : public QGraphicsItemGroup
    {
    public:
        AnimationTimelinePlayheadItem(const VadonEditor::UI::AnimationEditor& anim_editor, QGraphicsItem* parent = nullptr)
            : QGraphicsItemGroup(parent)
            , m_anim_editor(anim_editor)
            , m_triangle(nullptr)
            , m_line(nullptr)
            , m_dragging(false)
        {
            setFlags(QGraphicsItem::GraphicsItemFlag::ItemIsMovable
                | QGraphicsItem::GraphicsItemFlag::ItemSendsScenePositionChanges
                | QGraphicsItem::GraphicsItemFlag::ItemIgnoresTransformations);

            constexpr int c_triangle_height = 15;
            constexpr int c_triangle_width = 15;

            const QPolygonF triangle = QPolygonF{
                {
                    QPointF(-c_triangle_width / 2, -c_triangle_height),
                    QPointF(c_triangle_width / 2, -c_triangle_height),
                    QPointF(0, 0)
                }
            };

            m_triangle = new QGraphicsPolygonItem(triangle, this);
            m_triangle->setPen(Qt::PenStyle::NoPen);
            //m_triangle->setPos(0, m_theme.constants.top_margin - c_triangle_height);
            m_triangle->setBrush(m_anim_editor.get_theme().colors.playhead_color);

            m_line = new QGraphicsLineItem(QLineF(0, 0, 0, calculate_height()), this);

            QPen line_pen;
            line_pen.setColor(m_anim_editor.get_theme().colors.playhead_color);
            line_pen.setWidth(4);

            m_line->setPen(line_pen);
        }

        qreal calculate_height() const
        {
            return m_anim_editor.calculate_scene_height() - m_anim_editor.get_theme().constants.top_margin;
        }

        void scene_height_updated()
        {
            m_line->setLine(QLineF(0, 0, 0, calculate_height()));
            update();
        }

        void view_transform_updated()
        {
            QTransform line_transform;
            line_transform.scale(1, m_anim_editor.get_vertical_zoom());

            m_line->setTransform(line_transform);
        }

        void mousePressEvent(QGraphicsSceneMouseEvent* event) override
        {
            m_dragging = true;
            QGraphicsItem::mousePressEvent(event);
        }

        void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override
        {
            QGraphicsItem::mouseMoveEvent(event);
            update_play_head();
        }

        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override
        {
            update_play_head();
            m_dragging = false;

            QGraphicsItem::mouseReleaseEvent(event);
        }

        QVariant itemChange(GraphicsItemChange change, const QVariant& value) override
        {
            if (change == QGraphicsItem::GraphicsItemChange::ItemPositionChange)
            {
                QPointF new_pos = value.toPointF();
                new_pos.setY(m_anim_editor.get_theme().constants.top_margin); // Clamp Y position
                return new_pos;
            }

            return QGraphicsItem::itemChange(change, value);
        }
    private:
        void update_play_head()
        {
            // TODO: notify timeline to set time!
        }

        const VadonEditor::UI::AnimationEditor& m_anim_editor;

        QGraphicsPolygonItem* m_triangle;
        QGraphicsLineItem* m_line;
        bool m_dragging;
    };

    class AnimationChannelHeaderItem : public QGraphicsItemGroup
    {
    public:
        AnimationChannelHeaderItem(const VadonEditor::UI::AnimationChannelData& channel_data, const VadonEditor::UI::AnimationEditorTheme& theme, QGraphicsItem* parent = nullptr)
            : QGraphicsItemGroup(parent)
            , m_channel_data(channel_data)
            , m_theme(theme)
            , m_background(nullptr)
            , m_text(nullptr)
        {
            m_background = new QGraphicsRectItem(this);
            m_background->setRect(0, 0, m_theme.constants.left_margin, m_theme.constants.default_track_height);
            m_background->setBrush(m_theme.colors.channel_header_background);
            m_background->setZValue(0);

            m_text = new QGraphicsSimpleTextItem(this);
            m_text->setFlags(QGraphicsItem::GraphicsItemFlag::ItemIgnoresTransformations);
            m_text->setPen(m_theme.colors.channel_header_text);
            m_text->setFont(QFont("Sans", 10));
            m_text->setZValue(1);

            m_text->setText(channel_data.get_name());

            const QRectF background_rect = m_background->boundingRect();
            const QRectF text_rect = m_text->boundingRect();
            m_text->setPos((background_rect.width() - text_rect.width()) * 0.5, (background_rect.height() - text_rect.height()) * 0.5);

            m_lane_border = new QGraphicsLineItem(this);
            m_lane_border->setPos(0, m_theme.constants.default_track_height + (m_theme.constants.track_spacing * 0.5));
            m_lane_border->setPen(m_theme.colors.channel_lane_border);
            m_lane_border->setLine(0, 0, m_theme.constants.left_margin, 0);
            m_lane_border->setZValue(2);
        }

    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
        {
            // TODO: implement context menu to edit/remove item!
            QGraphicsItemGroup::contextMenuEvent(event);
        }
    private:
        const VadonEditor::UI::AnimationChannelData& m_channel_data;
        const VadonEditor::UI::AnimationEditorTheme& m_theme;

        QGraphicsRectItem* m_background;
        QGraphicsSimpleTextItem* m_text;
        QGraphicsLineItem* m_lane_border;
    };

    class AnimationKeyItem : public QGraphicsPolygonItem
    {
    public:
        AnimationKeyItem(const VadonEditor::UI::AnimationEditor& anim_editor, QGraphicsItem* parent = nullptr)
            : QGraphicsPolygonItem(parent)
            , m_anim_editor(anim_editor)
            , m_dragging(false)
            , m_fixed_y(0.0)
        {
            setFlags(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable | QGraphicsItem::GraphicsItemFlag::ItemIsMovable | QGraphicsItem::GraphicsItemFlag::ItemIgnoresTransformations | QGraphicsItem::GraphicsItemFlag::ItemSendsScenePositionChanges);

            constexpr qreal c_key_polygon_size = 15;

            const QPolygonF key_polygon = QPolygonF{
                {
                    QPointF(-c_key_polygon_size * 0.5, 0),
                    QPointF(0, -c_key_polygon_size * 0.5),
                    QPointF(c_key_polygon_size * 0.5, 0),
                    QPointF(0, c_key_polygon_size * 0.5)
                }
            };

            setBrush(m_anim_editor.get_theme().colors.key_fill);
            setPen(m_anim_editor.get_theme().colors.key_border);

            setPolygon(key_polygon);
        }

        QVariant itemChange(GraphicsItemChange change, const QVariant& value) override
        {
            if (change == QGraphicsItem::GraphicsItemChange::ItemPositionChange)
            {
                QPointF new_pos = value.toPointF();
                new_pos.setY(m_fixed_y);

                return new_pos;
            }

            return QGraphicsItem::itemChange(change, value);
        }

        void mousePressEvent(QGraphicsSceneMouseEvent* event) override
        {
            m_dragging = true;
            QGraphicsItem::mousePressEvent(event);
        }

        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override
        {
            m_dragging = false;
            QGraphicsItem::mouseReleaseEvent(event);
        }

        void set_fixed_y(qreal fixed_y) { m_fixed_y = fixed_y; }
    private:
        const VadonEditor::UI::AnimationEditor& m_anim_editor;

        bool m_dragging;
        qreal m_fixed_y;
    };
}
namespace VadonEditor::UI
{
    struct AnimationEditor::InternalGraphics
    {
        struct AnimationChannel
        {
            AnimationChannelHeaderItem* header = nullptr;
            QGraphicsRectItem* lane = nullptr;
            QList<QGraphicsItem*> keys;
        };

        QGraphicsRectItem* m_header_root;
        QGraphicsRectItem* m_timeline_root;
        AnimationTimelineRulerItem* m_ruler;
        AnimationTimelinePlayheadItem* m_playhead;

        QList<AnimationChannel> m_channels;

        void initialize(AnimationEditor& anim_editor)
        {
            QGraphicsView* timeline_view = anim_editor.m_ui.timelineView;

            const qreal timeline_width = c_animation_editor_scene_width - anim_editor.get_theme().constants.left_margin;

            m_timeline_root = new QGraphicsRectItem();
            m_timeline_root->setZValue(0);
            m_timeline_root->setBrush(QBrush());
            m_timeline_root->setPen(QPen());
            m_timeline_root->setRect(0, 0, 1, 1);
            m_timeline_root->update();

            timeline_view->scene()->addItem(m_timeline_root);

            m_header_root = new QGraphicsRectItem();
            m_header_root->setZValue(1);
            m_header_root->setBrush(QBrush());
            m_header_root->setPen(QPen());
            m_header_root->setRect(0, 0, anim_editor.get_theme().constants.left_margin, anim_editor.get_theme().constants.top_margin);
            m_header_root->update();

            timeline_view->scene()->addItem(m_header_root);

            m_ruler = new AnimationTimelineRulerItem(anim_editor);
            m_ruler->setZValue(2);
            m_ruler->setPos(anim_editor.get_theme().constants.left_margin, 0);

            timeline_view->scene()->addItem(m_ruler);

            m_playhead = new AnimationTimelinePlayheadItem(anim_editor);
            m_playhead->setZValue(1000);
            m_playhead->setPos(anim_editor.get_theme().constants.left_margin, 0);

            timeline_view->scene()->addItem(m_playhead);

            int y_offset = 0;
            for (const AnimationChannelData& channel_data : anim_editor.get_channel_data())
            {
                AnimationChannel anim_channel;

                anim_channel.header = new AnimationChannelHeaderItem(channel_data, anim_editor.get_theme(), m_header_root);
                anim_channel.header->setPos(0, y_offset);

                anim_channel.lane = new QGraphicsRectItem(m_timeline_root);
                anim_channel.lane->setZValue(50);

                // TODO: pick lane color based on channel type
                anim_channel.lane->setBrush(anim_editor.get_theme().colors.channel_lane_background_1);
                anim_channel.lane->setPen(anim_editor.get_theme().colors.channel_lane_border);

                anim_channel.lane->setRect(0, y_offset, timeline_width, channel_data.get_height());

                const qreal fixed_y_offset = y_offset + (channel_data.get_height() * 0.5);
                for (const AnimationKeyData& current_key : channel_data.get_keys())
                {
                    QGraphicsItem* key_item = create_key_item(anim_editor, current_key, fixed_y_offset);

                    const qreal x_offset = current_key.start_time * timeline_width;

                    key_item->setPos(x_offset, fixed_y_offset);
                    key_item->setZValue(100);

                    key_item->update();
                }
                y_offset += channel_data.get_height() + 2;
            }
        }

        void transform_updated(AnimationEditor& anim_editor)
        {
            QTransform dynamic_transform;
            dynamic_transform.translate(anim_editor.get_theme().constants.left_margin, anim_editor.get_theme().constants.top_margin);
            dynamic_transform.scale(anim_editor.get_horizontal_zoom(), anim_editor.get_vertical_zoom());

            m_timeline_root->setTransform(dynamic_transform);

            QTransform header_transform;
            header_transform.translate(0, anim_editor.get_theme().constants.top_margin);
            header_transform.scale(1.0, anim_editor.get_vertical_zoom());

            m_header_root->setTransform(header_transform);

            m_playhead->view_transform_updated();

            m_ruler->update();
        }

        void set_time(AnimationEditor& anim_editor, qreal time)
        {
            m_playhead->setPos(anim_editor.get_theme().constants.left_margin + c_animation_editor_scene_width * time * anim_editor.get_horizontal_zoom(), 0);
            m_playhead->update();
        }

        QGraphicsItem* create_key_item(AnimationEditor& anim_editor, const AnimationKeyData& key_data, qreal y_offset)
        {
            // TODO: branch on type!
            Q_UNUSED(key_data);

            AnimationKeyItem* key_item = new AnimationKeyItem(anim_editor, m_timeline_root);
            key_item->set_fixed_y(y_offset);
            return key_item;
        }
    };

    AnimationEditor::AnimationEditor(QWidget* parent)
        : QWidget(parent)
        , m_internal_graphics(std::make_unique<InternalGraphics>())
        , m_horizontal_zoom(1.0)
        , m_vertical_zoom(1.0)
        , m_time(0.0)
    {
        setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

        m_ui.setupUi(this);

        // Use default dark theme
        // FIXME: allow overriding the theme?
        m_theme.colors = c_animation_editor_dark_theme;

        initialize();
    }

    AnimationEditor::~AnimationEditor() = default;

    qreal AnimationEditor::calculate_scene_height() const
    {
        qreal height = m_theme.constants.top_margin;
        for (const AnimationChannelData& current_channel : m_channel_data)
        {
            height += (current_channel.get_height() + m_theme.constants.track_spacing) * m_vertical_zoom;
        }

        return height;
    }

    void AnimationEditor::playback_start_triggered()
    {
        if (m_timer.isActive() == false)
        {
            m_timer.start(16);
        }
    }

    void AnimationEditor::playback_stop_triggered()
    {
        if (m_timer.isActive() == true)
        {
            m_timer.stop();
        }
        else
        {
            reset_playback();
        }
    }

    void AnimationEditor::add_channel_triggered()
    {

    }

    void AnimationEditor::timeline_zoom_changed()
    {
        // Horizontal zoom
        {
            const qreal norm_h_zoom = qreal(m_ui.timelineHorizontalZoomSlider->value()) / 100.0;

            m_horizontal_zoom = 1.0 + (norm_h_zoom * 9.0);
        }

        // Vertical zoom
        {
            const qreal norm_v_zoom = qreal(m_ui.timelineVerticalZoomSlider->value()) / 100.0;

            if (norm_v_zoom < 0.5)
            {
                m_vertical_zoom = 2.0 * qMax(norm_v_zoom, 0.05);
            }
            else
            {
                m_vertical_zoom = 1.0 + ((norm_v_zoom - 0.5) * 4.0f);
            }
        }

        transform_updated();
    }

    void AnimationEditor::initialize()
    {
        {
            const QPixmap zoom_in_pixmap = QIcon::fromTheme(QIcon::ThemeIcon::ZoomIn).pixmap(QSize(16, 16));
            m_ui.timelineHorizontalZoomInLabel->setPixmap(zoom_in_pixmap);
            m_ui.timelineVerticalZoomInLabel->setPixmap(zoom_in_pixmap);

            const QPixmap zoom_out_pixmap = QIcon::fromTheme(QIcon::ThemeIcon::ZoomOut).pixmap(QSize(16, 16));
            m_ui.timelineHorizontalZoomOutLabel->setPixmap(zoom_out_pixmap);
            m_ui.timelineVerticalZoomOutLabel->setPixmap(zoom_out_pixmap);
        }

        m_ui.timelineView->setScene(&m_graphics_scene);

        m_ui.timelineView->setRenderHint(QPainter::RenderHint::Antialiasing);
        m_ui.timelineView->setDragMode(QGraphicsView::DragMode::RubberBandDrag);
        m_ui.timelineView->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop);

        // FIXME: remove this!
        {
            AnimationChannelData channel_a("Channel A");
            channel_a.add_key(AnimationKeyData(5.0, 0.25, 0.1));
            channel_a.add_key(AnimationKeyData(6.0, 0.47, 0.12));
            channel_a.add_key(AnimationKeyData(2.0, 0.73, 0.17));

            m_channel_data.push_back(channel_a);

            AnimationChannelData channel_b("Channel B");
            channel_b.add_key(AnimationKeyData(321, 0.11, 0.23));
            channel_b.add_key(AnimationKeyData(67, 0.67, 0.22));

            m_channel_data.push_back(channel_b);
        }

        m_internal_graphics->initialize(*this);

        // Set initial zoom
        timeline_zoom_changed();

        connect(&m_timer, &QTimer::timeout, this, &AnimationEditor::playback_timer_update);
    }

    void AnimationEditor::transform_updated()
    {
        m_internal_graphics->transform_updated(*this);

        m_ui.timelineView->scene()->setSceneRect(m_ui.timelineView->scene()->itemsBoundingRect());
    }

    void AnimationEditor::time_updated()
    {
        m_internal_graphics->set_time(*this, m_time);
        m_ui.timelineView->viewport()->update();
    }

    void AnimationEditor::playback_timer_update()
    {
        m_time += (16.0 / 1000.0) * 0.3;
        if (m_time > 1.0)
        {
            m_time -= 1.0;
        }

        time_updated();
    }

    void AnimationEditor::reset_playback()
    {
        m_time = 0.0;
        time_updated();
    }
}