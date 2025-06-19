#ifndef VADON_PRIVATE_CORE_CORE_HPP
#define VADON_PRIVATE_CORE_CORE_HPP
#include <Vadon/Core/Core.hpp>
#include <Vadon/Core/CoreConfiguration.hpp>

#include <Vadon/Private/Core/File/FileSystem.hpp>
#include <Vadon/Private/Core/Task/TaskSystem.hpp>

#include <Vadon/Private/Render/RenderSystem.hpp>
#include <Vadon/Private/Render/GraphicsAPI/GraphicsAPI.hpp>

#include <Vadon/Private/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Private/Scene/SceneSystem.hpp>

namespace Vadon::Private::Core
{
	class EngineCore : public Vadon::Core::EngineCoreImplementation
	{
	public:
		EngineCore();
		~EngineCore();

		bool initialize(const Vadon::Core::CoreConfiguration& config) override;
		void shutdown() override;

		const Vadon::Core::CommonConfiguration& get_config() const override { return m_config.common_config; }
	private:
		Vadon::Core::CoreConfiguration m_config;

		FileSystem m_file_system;
		TaskSystem m_task_system;

		Render::RenderSystem m_render_system;
		Render::GraphicsAPIBase::Implementation m_graphics_api;

		Scene::ResourceSystem m_resource_system;
		Scene::SceneSystem m_scene_system;
	};
}
#endif