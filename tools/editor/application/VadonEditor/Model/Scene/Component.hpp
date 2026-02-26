#ifndef VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#define VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#include <QHash>
#include <QUuid>
#include <QVariant>
namespace VadonEditor::Model
{
	using ComponentID = QUuid;
	struct Component
	{
		QHash<QUuid, QVariant> properties;
	};
}
#endif