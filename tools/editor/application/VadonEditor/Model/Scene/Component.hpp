#ifndef VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#define VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#include <QHash>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	using ComponentID = QUuid;
	struct Component
	{
		QHash<QUuid, QVariant> properties;

		bool save_data(Core::Application& application, const QUuid& type_id, QVariant& data) const;
		QUuid load_data(Core::Application& application, const QVariant& data);
	};
}
#endif