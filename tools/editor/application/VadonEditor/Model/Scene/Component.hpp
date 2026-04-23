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
		QUuid type_id;
		QHash<QUuid, QVariant> properties;

		bool initialize(Core::Application& application);
		bool save_data(Core::Application& application, QVariant& data) const;
		bool load_data(Core::Application& application, const QVariant& data);
	};
}
#endif