#ifndef VADON_PRIVATE_CORE_CORE_HPP
#define VADON_PRIVATE_CORE_CORE_HPP
#include <Vadon/Core/CoreInterface.hpp>
#include <Vadon/Core/Logger.hpp>

#include <Vadon/Private/Core/Task/TaskSystem.hpp>

#include <Vadon/Private/Render/RenderSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/GraphicsAPI.hpp>

#include <Vadon/Private/Scene/SceneSystem.hpp>

namespace Vadon::Private::Core
{
	class EngineCore : public Vadon::Core::EngineCoreInterface
	{
	public:
		EngineCore();
		~EngineCore();

		bool initialize(const Vadon::Core::Configuration& config = Vadon::Core::Configuration()) override;
		void update() override;
		void shutdown() override;

		const Vadon::Core::Configuration& get_config() const override { return m_config; }
	private:
		Vadon::Core::Configuration m_config;

		TaskSystem m_task_system;

		Render::RenderSystem m_render_system;
		Render::GraphicsAPIBase::Implementation m_graphics_api;

		Scene::SceneSystem m_scene_system;
	};
}
#endif