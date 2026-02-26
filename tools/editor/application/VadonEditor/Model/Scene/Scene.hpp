#ifndef VADONEDITOR_MODEL_SCENE_SCENE_HPP
#define VADONEDITOR_MODEL_SCENE_SCENE_HPP
#include <VadonEditor/Model/Scene/Entity.hpp>
namespace VadonEditor::Model
{
	class Resource;
	class Scene
	{
	public:
	private:
		Resource* m_resource;
		Entity m_root_entity;
	};
}
#endif