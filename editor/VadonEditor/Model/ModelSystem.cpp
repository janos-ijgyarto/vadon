#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/Scene/SceneTree.hpp>

#include <Vadon/Core/Object/ObjectSystem.hpp>
#include <Vadon/Scene/Node/Node.hpp>

namespace VadonEditor::Model
{
	struct ModelSystem::Internal
	{
		SceneTree m_scene_tree;

		Internal(Core::Editor& editor)
			: m_scene_tree(editor)
		{
		}

		bool initialize()
		{
			if (m_scene_tree.initialize() == false)
			{
				return false;
			}

			return true;
		}

		void update()
		{
			// TODO
		}

		Vadon::Core::ObjectClassInfoList get_node_type_list(Core::Editor& editor) const
		{
			return editor.get_engine_core().get_system<Vadon::Core::ObjectSystem>().get_subclass_hierarchy<Vadon::Scene::Node>();
		}
	};

	ModelSystem::~ModelSystem() = default;

	SceneTree& ModelSystem::get_scene_tree()
	{
		return m_internal->m_scene_tree;
	}

	Vadon::Core::ObjectClassInfoList ModelSystem::get_node_type_list() const
	{
		return m_internal->get_node_type_list(m_editor);
	}

	ModelSystem::ModelSystem(Core::Editor& editor)
		: System(editor)
		, m_internal(std::make_unique<Internal>(editor))
	{

	}

	bool ModelSystem::initialize()
	{
		return m_internal->initialize();
	}

	void ModelSystem::update()
	{
		m_internal->update();
	}
}