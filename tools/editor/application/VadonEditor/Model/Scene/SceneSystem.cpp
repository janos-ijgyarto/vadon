#include <VadonEditor/Model/Scene/SceneSystem.hpp>

namespace VadonEditor::Model
{
	SceneSystem::SceneSystem(Core::Application& application)
		: m_application(application)
	{

	}

	bool SceneSystem::initialize()
	{
		return true;
	}
}