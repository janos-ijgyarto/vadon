#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <Vadon/Core/Object/ObjectSystem.hpp>
#include <Vadon/Scene/Node/Node.hpp>

namespace VadonEditor::Model
{
	struct ModelSystem::Internal
	{
		Core::Editor& m_editor;
		Vadon::Core::ObjectClassInfoList m_node_class_list;

		Internal(Core::Editor& editor)
			: m_editor(editor)
		{
		}

		bool initialize()
		{
			// TODO!!!
			return true;
		}

		void post_init()
		{
			rebuild_node_class_list();
		}

		void update()
		{
			// TODO
		}

		void rebuild_node_class_list()
		{
			m_node_class_list = m_editor.get_engine_core().get_system<Vadon::Core::ObjectSystem>().get_subclass_hierarchy<Vadon::Scene::Node>();
		}
	};

	ModelSystem::~ModelSystem() = default;

	const Vadon::Core::ObjectClassInfoList& ModelSystem::get_node_class_list() const
	{
		// FIXME: should have a check here for whether the list is up-to-date, rebuild if not
		return m_internal->m_node_class_list;
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

	void ModelSystem::post_init()
	{
		m_internal->post_init();
	}

	void ModelSystem::update()
	{
		m_internal->update();
	}
}