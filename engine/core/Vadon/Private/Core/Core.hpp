#ifndef VADON_PRIVATE_CORE_CORE_HPP
#define VADON_PRIVATE_CORE_CORE_HPP
#include <Vadon/Core/CoreInterface.hpp>
#include <Vadon/Core/Logger.hpp>

#include <Vadon/Private/Core/Object/ObjectSystem.hpp>
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

		void set_logger(Vadon::Core::Logger* logger) override;
		Vadon::Core::Logger& get_logger() override { return *m_logger; }
	private:
		class DefaultLogger;

		Vadon::Core::Configuration m_config;

		std::unique_ptr<DefaultLogger> m_default_logger;
		Vadon::Core::Logger* m_logger;

		ObjectSystem m_object_system;
		TaskSystem m_task_system;

		Render::RenderSystem m_render_system;
		Render::GraphicsAPIBase::Implementation m_graphics_api;

		Scene::SceneSystem m_scene_system;
	};
}
#endif