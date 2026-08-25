#ifndef VADONEDITOR_MODEL_ANIMATION_ANIMATION_HPP
#define VADONEDITOR_MODEL_ANIMATION_ANIMATION_HPP
#include <VadonEditor/Core/Data/Object.hpp>
#include <Vadon/Foundation/Model/Animation/Animation.hpp>
#include <QObject>
namespace VadonEditor::Core
{
    class Application;
}
namespace VadonEditor::Model
{
    using AnimationChannelType = ::Vadon::Foundation::AnimationChannelSchema::Type;

    struct AnimationKey
    {
        static constexpr int c_invalid_key = -1;

        int id = c_invalid_key;
        QVariant value;
        qreal start_time = 0.0;
        qreal duration = 0.0;

        const bool is_valid() const { return id != c_invalid_key; }

        bool operator<(const AnimationKey& other) const { return start_time < other.start_time; }
    };

    class AnimationChannel : public QObject
    {
        Q_OBJECT
    public:
        AnimationChannel(const QUuid& id, AnimationChannelType type)
            : m_id(id)
            , m_type(type)
            , m_key_id_counter(1)
        {}

        const QUuid& get_id() const { return m_id; }
        AnimationChannelType get_type() const { return m_type; }
        
        const QString& get_tag() const { return m_tag; }
        void set_tag(const QString& tag)
        {
            m_tag = tag; 
            emit(channel_edited(m_id));
        }

        int add_key();
        void remove_key(int id);

        AnimationKey get_key(int id) const;
        void set_key(const AnimationKey& key_data);

        const QList<AnimationKey>& get_keys() const { return m_keys; }
        void sort_keys();
    signals:
        void channel_edited(const QUuid& channel_id);
    private:
        QUuid m_id;        
        AnimationChannelType m_type;        
        QString m_tag;
        QList<AnimationKey> m_keys;

        int m_key_id_counter;
    };

    class Resource;

    class Animation : public QObject
    {
        Q_OBJECT
    public:
        Animation(Resource* animation_resource)
            : m_resource(animation_resource)
        {
        }

        ~Animation();

        Resource* get_resource() const { return m_resource; }

        bool load_data();
        void store_data();

        void save_to_resource();

        AnimationChannel* add_channel(AnimationChannelType type);
        AnimationChannel* find_channel(const QUuid& channel_id) const;
        void remove_channel(const QUuid& channel_id);

        QList<QUuid> get_channel_id_list() const;

        static QUuid get_animation_type_uuid();
        static bool is_animation_base_of_type(VadonEditor::Core::Application& application, const QUuid& type_id);
    signals:
        void channel_added(const QUuid& animation_id, const QUuid& channel_id);
        void channel_removed(const QUuid& animation_id, const QUuid& channel_id);
        void channel_edited(const QUuid& animation_id, const QUuid& channel_id);
    private slots:
        void internal_channel_edited(const QUuid& channel_id);
    private:
        void internal_add_channel(AnimationChannel* channel);
        void clear_channels();

        Model::Resource* m_resource;

        QList<AnimationChannel*> m_channels;
    };
}
#endif