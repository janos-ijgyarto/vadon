#ifndef VADON_SCENE_SCENESYSTEM_HPP
#define VADON_SCENE_SCENESYSTEM_HPP
#include <Vadon/Scene/Module.hpp>
#include <Vadon/Scene/Node/Node.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Utilities/Serialization/JSON/JSON.hpp>
namespace Vadon::Scene
{
	class SceneSystem : public SceneSystemBase<SceneSystem>
	{
	public:
		virtual SceneHandle create_scene(SceneInfo scene_info) = 0;
		virtual bool is_scene_valid(SceneHandle scene_handle) const = 0;
		virtual void remove_scene(SceneHandle scene_handle) = 0;

		virtual SceneInfo get_scene_info(SceneHandle scene_handle) const = 0;

		virtual bool set_scene_data(SceneHandle scene_handle, Node& root_node) = 0;
		virtual bool save_scene(SceneHandle scene_handle, Vadon::Utilities::JSON& writer) = 0; // FIXME: take path and write to file internally?
		virtual bool load_scene(SceneHandle scene_handle, Vadon::Utilities::JSONReader& reader) = 0; // FIXME: take path and read from file internally?

		virtual void update(float delta_time) = 0;

		const Node& get_root() const { return m_root_node; }
		Node& get_root() { return const_cast<Node&>(const_cast<const SceneSystem*>(this)->get_root()); }
	protected:
		SceneSystem(Core::EngineCoreInterface& core)
			: System(core)
			, m_root_node(core)
		{
			m_root_node.m_in_tree = true;
		}

		Node m_root_node;

		friend Node;
	};
}
#endif