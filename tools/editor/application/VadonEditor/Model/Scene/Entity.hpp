#ifndef VADONEDITOR_MODEL_SCENE_ENTITY_HPP
#define VADONEDITOR_MODEL_SCENE_ENTITY_HPP
#include <VadonEditor/Model/Scene/Component.hpp>
#include <QList>
namespace VadonEditor::Model
{
	using SceneID = QUuid;
	struct Entity
	{
		QString name;
		Entity* parent = nullptr;
		QList<Entity> children;
		QHash<ComponentID, Component> components;
		SceneID scene;
	};
}
#endif