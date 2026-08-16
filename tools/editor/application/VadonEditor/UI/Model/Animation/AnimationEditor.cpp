#include <VadonEditor/UI/Model/Animation/AnimationEditor.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/UI/Model/Property/Color.hpp>
#include <VadonEditor/UI/Model/Property/Numeric.hpp>
#include <VadonEditor/UI/Model/Property/Vector.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Model/Resource/Resource.hpp>

#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <QMenu>
#include <QPainter>
#include <QSlider>
#include <QToolBar>
#include <QVector2D>
namespace
{
    constexpr VadonEditor::UI::AnimationEditorConstants c_default_animation_editor_constants;
    constexpr int c_animation_editor_scene_width = 2000;

    constexpr int c_animation_playback_tick_ms = 16; // Timeout for QTimer that runs playback

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

    constexpr const char* c_animation_channel_type_names[static_cast<size_t>(VadonEditor::Model::AnimationChannelType::TYPE_COUNT)] = {
        "int32",
        "uint32",
        "float",
        "Vector2",
        "Vector2i",
        "Vector3",
        "Vector3i",
        "Vector4",
        "ColorRGBA"
    };

    constexpr const char* get_animation_channel_type_name(VadonEditor::Model::AnimationChannelType channel_type)
    {
        return c_animation_channel_type_names[static_cast<size_t>(channel_type)];
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
        AnimationChannelHeaderItem(VadonEditor::UI::AnimationEditor& anim_editor, const QUuid& channel_id, const VadonEditor::UI::AnimationEditorTheme& theme, QGraphicsItem* parent = nullptr)
            : QGraphicsItemGroup(parent)
            , m_anim_editor(anim_editor)
            , m_channel_id(channel_id)
            , m_theme(theme)
            , m_background(nullptr)
            , m_text(nullptr)
        {
            m_background = new QGraphicsRectItem(this);
            m_background->setRect(0, 0, m_theme.constants.left_margin, VadonEditor::UI::AnimationEditorConstants::c_default_channel_height);
            m_background->setBrush(m_theme.colors.channel_header_background);
            m_background->setZValue(0);

            m_text = new QGraphicsSimpleTextItem(this);
            m_text->setFlags(QGraphicsItem::GraphicsItemFlag::ItemIgnoresTransformations);
            m_text->setPen(m_theme.colors.channel_header_text);
            m_text->setFont(QFont("Sans", 10));
            m_text->setZValue(1);

            update_header_text();

            m_lane_border = new QGraphicsLineItem(this);
            m_lane_border->setPos(0, VadonEditor::UI::AnimationEditorConstants::c_default_channel_height + (m_theme.constants.channel_spacing * 0.5));
            m_lane_border->setPen(m_theme.colors.channel_lane_border);
            m_lane_border->setLine(0, 0, m_theme.constants.left_margin, 0);
            m_lane_border->setZValue(2);
        }

        const QUuid& get_channel_id() const { return m_channel_id; }

        qreal get_channel_height() const { return m_background->rect().height(); }

        void update_header_text()
        {
            VadonEditor::Model::AnimationChannel* channel_data = m_anim_editor.get_animation().find_channel(m_channel_id);
            Q_ASSERT_X(channel_data != nullptr, "AnimationChannelHeaderItem::AnimationChannelHeaderItem", "Cannot find channel");

            QString header_text = channel_data->get_tag();
            if (header_text.isEmpty() == true)
            {
                header_text = QString("Channel_%1").arg(VadonEditor::Utilities::uuid_to_base64_string(m_channel_id));
            }

            m_text->setText(header_text);

            const QRectF background_rect = m_background->boundingRect();
            const QRectF text_rect = m_text->boundingRect();
            m_text->setPos((background_rect.width() - text_rect.width()) * 0.5, (background_rect.height() - text_rect.height()) * 0.5);
        }

        void mousePressEvent(QGraphicsSceneMouseEvent* event) override
        {
            event->accept();
        }

        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override
        {
            m_anim_editor.channel_selected(m_channel_id);
            QGraphicsItem::mouseReleaseEvent(event);
        }
    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
        {
            // TODO: implement context menu to edit/remove item!
            QGraphicsItemGroup::contextMenuEvent(event);
        }
    private:
        VadonEditor::UI::AnimationEditor& m_anim_editor;
        QUuid m_channel_id;
        const VadonEditor::UI::AnimationEditorTheme& m_theme;

        QGraphicsRectItem* m_background;
        QGraphicsSimpleTextItem* m_text;
        QGraphicsLineItem* m_lane_border;
    };

    class AnimationKeyItem : public QGraphicsPolygonItem
    {
    public:
        AnimationKeyItem(VadonEditor::UI::AnimationEditor& anim_editor, const QUuid& channel_id, const VadonEditor::Model::AnimationKey& key_data, QGraphicsItem* parent = nullptr)
            : QGraphicsPolygonItem(parent)
            , m_anim_editor(anim_editor)
            , m_channel_id(channel_id)
            , m_key_id(key_data.id)
            , m_dragging(false)
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

        const QUuid& get_channel_id() const { return m_channel_id; }
        const int get_key_id() const { return m_key_id; }

        QVariant itemChange(GraphicsItemChange change, const QVariant& value) override
        {
            if (change == QGraphicsItem::GraphicsItemChange::ItemPositionChange)
            {
                QPointF new_pos = value.toPointF();

                const qreal y_offset = parentItem()->boundingRect().height() * 0.5;
                new_pos.setY(y_offset);

                update_animation_key(new_pos);
                m_anim_editor.key_moved(m_channel_id, m_key_id);

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

        void set_start_time(qreal value)
        {
            internal_set_start_time(value);

            const QPointF prev_pos = pos();
            const qreal timeline_width = m_anim_editor.get_timeline_width();

            setPos(value * timeline_width, prev_pos.y());
        }

        void set_key_value(const QVariant& value)
        {
            VadonEditor::Model::AnimationKey new_key_data = get_key_data();
            new_key_data.value = value;

            set_key_data(new_key_data);
        }

        VadonEditor::Model::AnimationKey get_key_data() const
        {
            VadonEditor::Model::AnimationChannel* channel_data = m_anim_editor.get_animation().find_channel(m_channel_id);
            Q_ASSERT_X(channel_data != nullptr, "AnimationKeyItem::get_key_data", "Cannot find channel");

            VadonEditor::Model::AnimationKey key_data = channel_data->get_key(m_key_id);
            Q_ASSERT_X(key_data.is_valid() == true, "AnimationKeyItem::get_key_data", "Cannot find key");

            return key_data;
        }

        qreal get_time_from_position(const QPointF& position)
        {
            const qreal timeline_width = m_anim_editor.get_timeline_width();

            return position.x() / timeline_width;
        }
    private:
        void update_animation_key(const QPointF& position)
        {
            internal_set_start_time(get_time_from_position(position));
        }

        void internal_set_start_time(qreal value)
        {
            VadonEditor::Model::AnimationKey new_key_data = get_key_data();

            new_key_data.start_time = value;
            set_key_data(new_key_data);
        }

        void set_key_data(const VadonEditor::Model::AnimationKey& key_data)
        {
            VadonEditor::Model::AnimationChannel* channel_data = m_anim_editor.get_animation().find_channel(m_channel_id);
            Q_ASSERT_X(channel_data != nullptr, "AnimationKeyItem::set_key_data", "Cannot find channel");

            channel_data->set_key(key_data);
        }

        VadonEditor::UI::AnimationEditor& m_anim_editor;
        QUuid m_channel_id;
        int m_key_id;

        bool m_dragging;
    };

    class AnimationChannelLaneItem : public QGraphicsRectItem
    {
    public:
        AnimationChannelLaneItem(VadonEditor::UI::AnimationEditor& anim_editor, const QUuid& channel_id, QGraphicsItem* parent = nullptr)
            : QGraphicsRectItem(parent)
            , m_anim_editor(anim_editor)
            , m_channel_id(channel_id)
        {
            // TODO: pick lane color based on channel type
            setBrush(m_anim_editor.get_theme().colors.channel_lane_background_1);
            setPen(m_anim_editor.get_theme().colors.channel_lane_border);

            const qreal timeline_width = anim_editor.get_timeline_width();
            setRect(0, 0, timeline_width, VadonEditor::UI::AnimationEditorConstants::c_default_channel_height);
        }

        const QUuid& get_channel_id() const { return m_channel_id; }

        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override
        {
            m_anim_editor.channel_context_menu_requested(m_channel_id, event->screenPos(), mapFromScene(event->scenePos()));
        }

        void create_key_item(const QPointF& position)
        {
            VadonEditor::Model::AnimationChannel* channel_data = m_anim_editor.get_animation().find_channel(m_channel_id);
            Q_ASSERT_X(channel_data != nullptr, "AnimationChannelLaneItem::create_key_item", "Cannot find channel");

            const int new_key_id = channel_data->add_key();
            Q_ASSERT_X(new_key_id != VadonEditor::Model::AnimationKey::c_invalid_key, "AnimationChannelLaneItem::create_key_item", "Failed to create key");

            VadonEditor::Model::AnimationKey key_data = channel_data->get_key(new_key_id);
            key_data.start_time = position.x() / rect().width();

            add_key_item(key_data);
        }

        void add_key_item(const VadonEditor::Model::AnimationKey& key_data)
        {
            QGraphicsItem* key_item = new AnimationKeyItem(m_anim_editor, m_channel_id, key_data, this);

            const qreal timeline_width = m_anim_editor.get_timeline_width();
            const qreal x_offset = key_data.start_time * timeline_width;

            key_item->setPos(x_offset, rect().height() * 0.5);
            key_item->setZValue(100);

            key_item->update();
        }
    private:
        VadonEditor::UI::AnimationEditor& m_anim_editor;
        QUuid m_channel_id;
    };
}
namespace VadonEditor::UI
{
    struct AnimationEditor::InternalGraphics
    {
        struct AnimationChannel
        {
            AnimationChannelHeaderItem* header = nullptr;
            AnimationChannelLaneItem* lane = nullptr;

            bool is_valid() const { return (header != nullptr) && (lane != nullptr); }
        };

        struct CreateKeyData
        {
            AnimationChannel channel;
            QPointF position;

            bool is_valid() const { return channel.is_valid() == true; }
            void clear()
            {
                channel = AnimationChannel();
                position = QPointF();
            }
        };

        QGraphicsRectItem* m_header_root;
        QGraphicsRectItem* m_timeline_root;
        AnimationTimelineRulerItem* m_ruler;
        AnimationTimelinePlayheadItem* m_playhead;

        QList<AnimationChannel> m_channels;

        QUuid m_selected_channel;
        AnimationKeyItem* m_selected_key = nullptr;

        CreateKeyData m_create_key_data;

        PropertyWidget* m_key_value_property_widget = nullptr;

        void initialize(AnimationEditor& anim_editor)
        {
            QGraphicsView* timeline_view = anim_editor.m_ui.timelineView;

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
            m_header_root->setRect(0, 0, 1, 1);
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

            for (int channel_type_index = 0; channel_type_index < static_cast<int>(Model::AnimationChannelType::TYPE_COUNT); ++channel_type_index)
            {
                QAction* add_channel_action = new QAction(&anim_editor);
                add_channel_action->setMenuRole(QAction::MenuRole::TextHeuristicRole);

                const Model::AnimationChannelType channel_type = static_cast<Model::AnimationChannelType>(channel_type_index);
                add_channel_action->setText(get_animation_channel_type_name(channel_type));

                add_channel_action->setData(channel_type_index);

                connect(add_channel_action, &QAction::triggered, &anim_editor, &AnimationEditor::add_channel_triggered);

                anim_editor.m_ui.addChannelButton->addAction(add_channel_action);

                set_channel_ui_enabled(anim_editor, false);
                set_key_properties_ui_enabled(anim_editor, false);
            }

            // Add all the channels
            for (const QUuid& current_channel_id : anim_editor.get_animation().get_channel_id_list())
            {
                add_channel(anim_editor, current_channel_id);
            }

            update_channel_layout();
        }

        void create_channel(AnimationEditor& anim_editor, Model::AnimationChannelType channel_type)
        {
            VadonEditor::Model::AnimationChannel* channel_data = anim_editor.m_animation.add_channel(channel_type);
            Q_ASSERT_X(channel_data != nullptr, "VadonEditor::UI::AnimationEditor::InternalGraphics::create_channel", "Failed to create channel");

            add_channel(anim_editor, channel_data->get_id());

            update_channel_layout();
        }

        void add_channel(AnimationEditor& anim_editor, const QUuid& channel_id)
        {
            AnimationChannel anim_channel;

            anim_channel.header = new AnimationChannelHeaderItem(anim_editor, channel_id, anim_editor.get_theme(), m_header_root);

            anim_channel.lane = new AnimationChannelLaneItem(anim_editor, channel_id, m_timeline_root);
            anim_channel.lane->setZValue(50);

            m_channels.push_back(anim_channel);

            // Add all the keys as well
            VadonEditor::Model::AnimationChannel* channel_data = anim_editor.get_animation().find_channel(channel_id);
            Q_ASSERT_X(channel_data != nullptr, "VadonEditor::UI::AnimationEditor::InternalGraphics::add_channel", "Cannot find channel");

            for (const Model::AnimationKey& current_key : channel_data->get_keys())
            {
                anim_channel.lane->add_key_item(current_key);
            }
        }

        AnimationChannel find_channel(const QUuid& channel_id)
        {
            for (const AnimationChannel& current_channel : m_channels)
            {
                if (current_channel.lane->get_channel_id() == channel_id)
                {
                    return current_channel;
                }
            }

            return AnimationChannel();
        }

        void update_channel_layout()
        {
            qreal y_offset = 0;
            for (InternalGraphics::AnimationChannel& current_channel : m_channels)
            {
                current_channel.header->setPos(0, y_offset);
                current_channel.lane->setPos(0, y_offset);

                y_offset += current_channel.header->get_channel_height() + 2;
            }

            m_playhead->scene_height_updated();
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

        void set_channel_ui_enabled(AnimationEditor& anim_editor, bool enabled)
        {
            anim_editor.m_ui.removeChannelButton->setEnabled(enabled);

            anim_editor.m_ui.channelPropertiesGroup->setVisible(enabled);
            anim_editor.m_ui.channelPropertiesGroup->setEnabled(enabled);
        }

        void update_channel_properties_ui(AnimationEditor& anim_editor, const QUuid& channel_id)
        {
            const Model::AnimationChannel* animation_channel = anim_editor.get_animation().find_channel(channel_id);
            Q_ASSERT_X(animation_channel != nullptr, "VadonEditor::UI::AnimationEditor::InternalGraphics::update_channel_properties_ui", "Cannot find channel");

            anim_editor.m_ui.channelIDTextLabel->setText(Utilities::uuid_to_base64_string(channel_id));
            anim_editor.m_ui.channelTypeTextLabel->setText(get_animation_channel_type_name(animation_channel->get_type()));

            // Block signals so we don't get an infinite loop
            anim_editor.m_ui.channelTagLineEdit->blockSignals(true);
            anim_editor.m_ui.channelTagLineEdit->setText(animation_channel->get_tag());
            anim_editor.m_ui.channelTagLineEdit->blockSignals(false);
        }

        Model::AnimationChannel* get_selected_channel(Model::Animation& animation) const
        {
            if (Utilities::is_uuid_valid(m_selected_channel) == false)
            {
                return nullptr;
            }

            return animation.find_channel(m_selected_channel);
        }

        void set_selected_channel(AnimationEditor& anim_editor, const QUuid& channel_id)
        {
            if (m_selected_channel == channel_id)
            {
                // Nothing to do
                return;
            }

            if (Utilities::is_uuid_valid(m_selected_channel) == true)
            {
                if (Utilities::is_uuid_valid(channel_id) == false)
                {
                    // Channel deselected, disable the UI
                    set_channel_ui_enabled(anim_editor, false);
                }
                else
                {
                    // Update the UI with newly selected channel
                    update_channel_properties_ui(anim_editor, channel_id);
                }
            }
            else
            {
                // Deselect keys
                clear_key_selection(anim_editor);

                // Enable the UI for newly selected channel
                set_channel_ui_enabled(anim_editor, true);
                update_channel_properties_ui(anim_editor, channel_id);
            }

            m_selected_channel = channel_id;
        }

        void item_selection_changed(AnimationEditor& anim_editor)
        {
            QList<QGraphicsItem*> selected_items = anim_editor.m_graphics_scene.selectedItems();
            if (selected_items.isEmpty() == true)
            {
                // No keys are selected
                set_key_selection(anim_editor, nullptr);
                return;
            }

            // We selected keys, deselect channel
            set_selected_channel(anim_editor, QUuid());

            if (selected_items.count() == 1)
            {
                AnimationKeyItem* key_item = static_cast<AnimationKeyItem*>(selected_items.front());
                set_key_selection(anim_editor, key_item);
            }
            else
            {
                // Unset key properties UI when multiple keys are selected
                set_key_selection(anim_editor, nullptr);
            }
        }

        void set_key_properties_ui_enabled(AnimationEditor& anim_editor, bool enabled)
        {
            anim_editor.m_ui.keyPropertiesGroup->setVisible(enabled);
            anim_editor.m_ui.keyPropertiesGroup->setEnabled(enabled);
        }

        void update_key_properties_ui(AnimationEditor& anim_editor, AnimationKeyItem* key_item)
        {
            const Model::AnimationKey anim_key_data = key_item->get_key_data();

            anim_editor.m_ui.keyTimeSpinBox->blockSignals(true);
            anim_editor.m_ui.keyTimeSpinBox->setValue(anim_key_data.start_time);
            anim_editor.m_ui.keyTimeSpinBox->blockSignals(false);

            // Remove previous row
            anim_editor.m_ui.keyPropertiesLayout->removeRow(1);

            const Model::AnimationChannel* anim_channel = anim_editor.get_animation().find_channel(key_item->get_channel_id());
            Q_ASSERT_X(anim_channel != nullptr, "VadonEditor::UI::AnimationEditor::InternalGraphics::update_key_properties_ui", "Cannot find channel");

            const QUuid animation_id = anim_editor.get_animation().get_resource()->get_info().id;
            switch (anim_channel->get_type())
            {
                case Model::AnimationChannelType::INT:
                {
                    m_key_value_property_widget = new PropertySpinBox(animation_id, anim_key_data.value.toInt(), true, nullptr);
                }
                break;
                case Model::AnimationChannelType::UINT:
                {
                    m_key_value_property_widget = new PropertySpinBox(animation_id, anim_key_data.value.toUInt(), false, nullptr);
                }
                break;
                case Model::AnimationChannelType::FLOAT:
                {
                    m_key_value_property_widget = new PropertyDoubleSpinBox(animation_id, anim_key_data.value.toFloat(), nullptr);
                }
                break;
                case Model::AnimationChannelType::VECTOR2:
                {
                    m_key_value_property_widget = new PropertyVector2(animation_id, anim_key_data.value.value<QVector2D>(), nullptr);
                }
                break;
                case Model::AnimationChannelType::VECTOR2I:
                {
                    m_key_value_property_widget = new PropertyVector2i(animation_id, anim_key_data.value.toPoint(), nullptr);
                }
                break;
                case Model::AnimationChannelType::VECTOR3:
                {
                    m_key_value_property_widget = new PropertyVector3(animation_id, anim_key_data.value.value<QVector3D>(), nullptr);
                }
                break;
                case Model::AnimationChannelType::VECTOR3I:
                {
                    m_key_value_property_widget = new PropertyVector3i(animation_id, anim_key_data.value.toLine(), nullptr);
                }
                break;
                case Model::AnimationChannelType::VECTOR4:
                {
                    m_key_value_property_widget = new PropertyVector4(animation_id, anim_key_data.value.value<QVector4D>(), nullptr);
                }
                break;
                case Model::AnimationChannelType::COLORRGBA:
                {
                    m_key_value_property_widget = new PropertyColorRGBA(animation_id, anim_key_data.value.value<QColor>(), nullptr);
                }
                break;
            }

            connect(m_key_value_property_widget, &PropertyWidget::value_changed, &anim_editor, &AnimationEditor::key_value_changed);

            anim_editor.m_ui.keyPropertiesLayout->addRow(new QLabel("Value:"), m_key_value_property_widget);
        }

        void key_item_moved(AnimationEditor& anim_editor, const QUuid& channel_id, int key_id)
        {
            if (m_selected_key != nullptr)
            {
                if ((m_selected_key->get_channel_id() == channel_id) && (m_selected_key->get_key_id() == key_id))
                {
                    // We are moving the same key as the one being edited
                    anim_editor.m_ui.keyTimeSpinBox->blockSignals(true);
                    anim_editor.m_ui.keyTimeSpinBox->setValue(m_selected_key->get_time_from_position(m_selected_key->pos()));
                    anim_editor.m_ui.keyTimeSpinBox->blockSignals(false);
                }
            }
        }

        void set_key_selection(AnimationEditor& anim_editor, AnimationKeyItem* key_item)
        {
            if (m_selected_key == key_item)
            {
                // Nothing to do
                return;
            }

            if (m_selected_key != nullptr)
            {
                if (key_item == nullptr)
                {
                    // Keys deselected, disable the UI
                    set_key_properties_ui_enabled(anim_editor, false);
                }
                else
                {
                    // Update UI with newly selected key
                    update_key_properties_ui(anim_editor, key_item);
                }
            }
            else
            {
                // Enable the UI for newly selected key
                set_key_properties_ui_enabled(anim_editor, true);
                update_key_properties_ui(anim_editor, key_item);
            }

            m_selected_key = key_item;
        }

        void clear_key_selection(AnimationEditor& anim_editor)
        {
            set_key_selection(anim_editor, nullptr);

            if (anim_editor.m_graphics_scene.selectedItems().isEmpty() == true)
            {
                // Selection is already cleared
                return;
            }
            anim_editor.m_graphics_scene.clearSelection();
        }
    };

    AnimationEditor::AnimationEditor(Model::Resource* resource, QWidget* parent)
        : QWidget(parent)
        , m_resource(resource)
        , m_animation(resource)
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

    bool AnimationEditor::initialize()
    {
        if (m_animation.load_data() == false)
        {
            return false;
        }

        {
            const QPixmap zoom_in_pixmap = QIcon::fromTheme(QIcon::ThemeIcon::ZoomIn).pixmap(QSize(16, 16));
            m_ui.timelineHorizontalZoomInLabel->setPixmap(zoom_in_pixmap);
            m_ui.timelineVerticalZoomInLabel->setPixmap(zoom_in_pixmap);

            const QPixmap zoom_out_pixmap = QIcon::fromTheme(QIcon::ThemeIcon::ZoomOut).pixmap(QSize(16, 16));
            m_ui.timelineHorizontalZoomOutLabel->setPixmap(zoom_out_pixmap);
            m_ui.timelineVerticalZoomOutLabel->setPixmap(zoom_out_pixmap);
        }

        // Add toolbar for buttons
        QToolBar* toolbar = new QToolBar();
        toolbar->addAction(m_ui.actionSave);

        m_ui.mainLayout->setMenuBar(toolbar);

        m_ui.timelineView->setScene(&m_graphics_scene);

        m_ui.timelineView->setRenderHint(QPainter::RenderHint::Antialiasing);
        m_ui.timelineView->setDragMode(QGraphicsView::DragMode::RubberBandDrag);
        m_ui.timelineView->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop);

        m_internal_graphics->initialize(*this);

        // Set initial zoom
        timeline_zoom_changed();

        connect(&m_animation, &Model::Animation::channel_added, this, &AnimationEditor::animation_edited);
        connect(&m_animation, &Model::Animation::channel_removed, this, &AnimationEditor::animation_edited);
        connect(&m_animation, &Model::Animation::channel_edited, this, &AnimationEditor::animation_edited);

        connect(&m_timer, &QTimer::timeout, this, &AnimationEditor::playback_timer_update);

        connect(&m_graphics_scene, &QGraphicsScene::selectionChanged, this, &AnimationEditor::item_selection_changed);

        update_title();

        return true;
    }

    qreal AnimationEditor::get_timeline_width() const
    {
        return c_animation_editor_scene_width - get_theme().constants.left_margin;
    }

    qreal AnimationEditor::calculate_scene_height() const
    {
        qreal height = m_theme.constants.top_margin;
        for (const InternalGraphics::AnimationChannel& current_channel : m_internal_graphics->m_channels)
        {
            height += (current_channel.header->get_channel_height() + m_theme.constants.channel_spacing) * m_vertical_zoom;
        }

        return height;
    }

    void AnimationEditor::channel_selected(const QUuid& channel_id)
    {
        m_internal_graphics->set_selected_channel(*this, channel_id);
    }

    void AnimationEditor::key_moved(const QUuid& channel_id, int key_id)
    {
        m_internal_graphics->key_item_moved(*this, channel_id, key_id);
    }

    void AnimationEditor::channel_context_menu_requested(const QUuid& channel_id, const QPoint& screen_pos, const QPointF& item_pos)
    {
#ifndef QT_NO_CONTEXTMENU
        m_internal_graphics->m_create_key_data.channel = m_internal_graphics->find_channel(channel_id);
        m_internal_graphics->m_create_key_data.position = item_pos;

        QMenu menu(get_view());

        menu.addAction(m_ui.actionAddKey);
        
        if (m_graphics_scene.selectedItems().isEmpty() == false)
        {
            menu.addAction(m_ui.actionRemoveKeys);
        }

        menu.exec(screen_pos);
#else
        Q_UNUSED(event);
#endif
    }

    void AnimationEditor::playback_start_triggered()
    {
        if (m_timer.isActive() == false)
        {
            m_timer.start(c_animation_playback_tick_ms);
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
        QObject* sender_action_obj = sender();
        QAction* sender_action = qobject_cast<QAction*>(sender_action_obj);
        if (sender_action != nullptr)
        {
            const int channel_type_index = sender_action->data().toInt();
            m_internal_graphics->create_channel(*this, static_cast<Model::AnimationChannelType>(channel_type_index));
        }
    }

    void AnimationEditor::remove_channel_triggered()
    {
        Model::AnimationChannel* selected_channel = m_internal_graphics->get_selected_channel(m_animation);
        if (selected_channel == nullptr)
        {
            Q_ASSERT_X(false, "VadonEditor::UI::AnimationEditor::remove_channel_triggered", "Cannot remove channel with no channel selection");
            return;
        }

        // To play it safe, clear all selections before deleting
        m_internal_graphics->clear_key_selection(*this);
        m_internal_graphics->set_selected_channel(*this, QUuid());

        const QUuid channel_id = selected_channel->get_id();
        m_animation.remove_channel(channel_id);

        const InternalGraphics::AnimationChannel graphics_channel = m_internal_graphics->find_channel(channel_id);
        m_graphics_scene.removeItem(graphics_channel.header);
        m_graphics_scene.removeItem(graphics_channel.lane);

        delete graphics_channel.header;
        delete graphics_channel.lane;

        m_internal_graphics->update_channel_layout();
    }

    void AnimationEditor::add_key_triggered()
    {
        if (m_internal_graphics->m_create_key_data.is_valid() == false)
        {
            Q_ASSERT_X(false, "VadonEditor::UI::AnimationEditor::add_key_triggered", "Invalid state");
            return;
        }

        m_internal_graphics->m_create_key_data.channel.lane->create_key_item(m_internal_graphics->m_create_key_data.position);
    }

    void AnimationEditor::remove_keys_triggered()
    {
        QList<QGraphicsItem*> selected_items = m_graphics_scene.selectedItems();

        if (selected_items.isEmpty() == true)
        {
            // Nothing to delete
            return;
        }

        // To play it safe, clear all selections before deleting
        m_internal_graphics->clear_key_selection(*this);
        m_internal_graphics->set_selected_channel(*this, QUuid());

        for (QGraphicsItem* current_item : selected_items)
        {
            AnimationKeyItem* current_key_item = static_cast<AnimationKeyItem*>(current_item);

            Model::AnimationChannel* key_channel = m_animation.find_channel(current_key_item->get_channel_id());
            key_channel->remove_key(current_key_item->get_key_id());

            m_graphics_scene.removeItem(current_key_item);
            delete current_key_item;
        }
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

    void AnimationEditor::save_triggered()
    {
        m_animation.save_to_resource();
        update_title();
    }

    void AnimationEditor::animation_edited()
    {
        // FIXME: at the moment we will update on every change
        // Implement buffering to make sure we don't spam repeated updates
        // and/or find a way to only update a very specific part and minimize the overhead
        m_animation.store_data();
        update_title();
    }

    void AnimationEditor::animation_name_changed(const QString& text)
    {
        m_animation.get_resource()->set_property(Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ResourceSchema::c_name_property.id), text);
        update_title();
    }

    void AnimationEditor::channel_tag_text_changed(const QString& text)
    {        
        Model::AnimationChannel* selected_channel = m_internal_graphics->get_selected_channel(m_animation);

        if (selected_channel == nullptr)
        {
            Q_ASSERT_X(false, "VadonEditor::UI::AnimationEditor::channel_tag_text_changed", "Should not receive tag update when no channel is selected");
            return;
        }

        selected_channel->set_tag(text);

        const InternalGraphics::AnimationChannel channel_item = m_internal_graphics->find_channel(m_internal_graphics->m_selected_channel);
        if (channel_item.is_valid() == false)
        {
            Q_ASSERT_X(false, "VadonEditor::UI::AnimationEditor::channel_tag_text_changed", "Cannot find channel items");
            return;
        }

        channel_item.header->update_header_text();
    }

    void AnimationEditor::key_time_changed(double value)
    {
        if (m_internal_graphics->m_selected_key == nullptr)
        {
            Q_ASSERT_X(false, "VadonEditor::UI::AnimationEditor::key_time_changed", "Should not receive key update when no key is selected");
            return;
        }

        m_internal_graphics->m_selected_key->set_start_time(value);
    }

    void AnimationEditor::key_value_changed()
    {
        if (m_internal_graphics->m_selected_key == nullptr)
        {
            Q_ASSERT_X(false, "VadonEditor::UI::AnimationEditor::key_value_changed", "Should not receive key update when no key is selected");
            return;
        }

        Q_ASSERT_X(m_internal_graphics->m_key_value_property_widget != nullptr, "VadonEditor::UI::AnimationEditor::key_value_changed", "Property widget not set!");

        m_internal_graphics->m_selected_key->set_key_value(m_internal_graphics->m_key_value_property_widget->get_value());
    }

    void AnimationEditor::item_selection_changed()
    {
        m_internal_graphics->item_selection_changed(*this);
    }

    void AnimationEditor::update_title()
    {
        Model::Resource* anim_resource = m_animation.get_resource();
        Core::Application& application = anim_resource->get_application();

        const int anim_asset_id = application.get_model_system().get_resource_system().find_resource_asset_id(anim_resource->get_info().id);
        Q_ASSERT_X(anim_asset_id != Core::AssetInfo::c_invalid_file_id, "VadonEditor::UI::AnimationEditor::update_title", "Cannot find resource asset");

        const Core::AssetManager& asset_manager = application.get_asset_manager();
        const QModelIndex asset_index = asset_manager.find_asset_index(anim_asset_id);
        Q_ASSERT_X(asset_index.isValid(), "VadonEditor::UI::AnimationEditor::update_title", "Cannot find resource asset");

        const Core::AssetInfo animation_asset_info = asset_manager.get_asset_info(asset_index);

        QString animation_title = animation_asset_info.path;
        if (anim_resource->is_modified() == true)
        {
            animation_title += " (*)";
        }

        setWindowTitle(QString("Animation Editor - %1").arg(animation_title));
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
        constexpr qreal c_playback_timestep = (qreal(c_animation_playback_tick_ms) / 1000.0) * 0.3;

        m_time += c_playback_timestep;
        if (m_time > 1.0)
        {
            // Wrap back around
            // TODO: implement settings for looping, one-shot, etc.
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