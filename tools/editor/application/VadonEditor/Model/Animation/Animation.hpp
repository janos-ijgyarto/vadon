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
    using AnimationChannelSampleMethod = ::Vadon::Foundation::AnimationChannelSchema::SampleMethod;

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

    // FIXME: this is a hacky solution, find a more robust way to map between properties and channels!
    struct AnimationChannelMetadata
    {
        QUuid resource_type; // NOTE: only valid for UUID channel!
    };

    class AnimationChannel : public QObject
    {
        Q_OBJECT
    public:
        AnimationChannel(const QUuid& id, AnimationChannelType type, const AnimationChannelMetadata& metadata)
            : m_id(id)
            , m_type(type)
            , m_sample_method(AnimationChannelSampleMethod::LINEAR)
            , m_metadata(metadata)
            , m_key_id_counter(1)
        {}

        const QUuid& get_id() const { return m_id; }
        AnimationChannelType get_type() const { return m_type; }

        AnimationChannelSampleMethod get_sample_method() const { return m_sample_method; }
        void set_sample_method(AnimationChannelSampleMethod sample_method);
        
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

        const AnimationChannelMetadata& get_metadata() const { return m_metadata; }
        void set_metadata(const AnimationChannelMetadata& metadata) { m_metadata = metadata; }

        static QUuid get_sample_method_id(AnimationChannelSampleMethod sample_method);
        static AnimationChannelSampleMethod get_sample_method_from_id(const QUuid& sample_method_id);
    signals:
        void channel_edited(const QUuid& channel_id);
    private:
        QUuid m_id;        
        AnimationChannelType m_type;
        AnimationChannelSampleMethod m_sample_method;
        QString m_tag;
        QList<AnimationKey> m_keys;

        AnimationChannelMetadata m_metadata;

        int m_key_id_counter;
    };

    class Resource;

    class Animation : public QObject
    {
        Q_OBJECT
    public:
        Animation(Core::Application& application);
        ~Animation();

        Core::Application& get_application() const { return m_application; }

        QUuid get_id() const { return m_id; }

        // TODO: load from raw data?
        bool store_data(QVariant& data) const;

        bool load_from_resource(const Model::Resource* resource);
        bool store_to_resource(Model::Resource* resource) const;

        AnimationChannel* add_channel(AnimationChannelType type, const AnimationChannelMetadata& metadata = AnimationChannelMetadata{});
        AnimationChannel* find_channel(const QUuid& channel_id) const;
        void remove_channel(const QUuid& channel_id);

        QList<QUuid> get_channel_id_list() const;

        static QUuid get_animation_type_uuid();
        static bool is_animation_base_of_type(VadonEditor::Core::Application& application, const QUuid& type_id);

        static QUuid get_animation_library_type_uuid();
        static QUuid get_entity_animation_type_uuid();
        static QUuid get_entity_animation_entry_type_uuid();

        static QUuid get_animation_component_type_uuid();
    signals:
        void channel_added(const QUuid& animation_id, const QUuid& channel_id);
        void channel_removed(const QUuid& animation_id, const QUuid& channel_id);
        void channel_edited(const QUuid& animation_id, const QUuid& channel_id);
    private slots:
        void internal_channel_edited(const QUuid& channel_id);
    private:
        void internal_add_channel(AnimationChannel* channel);
        void clear_channels();

        Core::Application& m_application;
        QUuid m_id;
        QList<AnimationChannel*> m_channels;
    };
}
#endif