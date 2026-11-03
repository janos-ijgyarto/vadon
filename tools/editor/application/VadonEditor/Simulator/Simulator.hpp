#ifndef VADONEDITOR_SIMULATOR_SIMULATOR_HPP
#define VADONEDITOR_SIMULATOR_SIMULATOR_HPP
#include <Vadon/Foundation/Editor/Simulator/SimulatorInterface.hpp>
#include <memory>
#include <QUuid>
#include <QVariant>
namespace Vadon::Foundation
{
	class EditorSimulatorPluginInterface;
}
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Simulator
{
	struct SimulatorStartupOptions
	{
		bool debug_break_on_init = false;
		QString configuration_name;
	};

	struct SimulatorSetting
	{
		QUuid id;
		QUuid type;
		QString label;
		QVariant value;
	};

	struct SimulatorSettingsData
	{
		// TODO: anything else?
		QList<SimulatorSetting> settings;

		void clear()
		{
			settings.clear();
		}
	};

	// TODO: "hide" the simulator interface so it's only available internally?
	class Simulator : public Vadon::Foundation::EditorSimulatorInterface
	{
	public:
		~Simulator();

		::Vadon::Foundation::EditorSimulatorPluginInterface* get_plugin_interface() const;

		void dispatch_message_to_editor(const char* data, size_t size) override;

		::Vadon::Foundation::SimulatorToolchainConfiguration get_toolchain_configuration() const override;

		const SimulatorSettingsData& get_settings() const;
		void set_setting_value(const QUuid& id, const QVariant& value);

		bool run_simulator(const SimulatorStartupOptions& startup_options);
		bool is_running() const;
		void stop_simulator();
	private:
		Simulator(Core::Application& application);

		bool initialize();
		void shutdown();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Application;
	};
}
#endif