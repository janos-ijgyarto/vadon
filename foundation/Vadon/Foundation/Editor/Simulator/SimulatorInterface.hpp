#ifndef VADON_FOUNDATION_EDITOR_SIMULATOR_SIMULATORINTERFACE_HPP
#define VADON_FOUNDATION_EDITOR_SIMULATOR_SIMULATORINTERFACE_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
namespace Vadon
{
	namespace Foundation
	{
		struct SimulatorToolchainConfiguration
		{
			const char* temp_path;
		};

		struct SimulatorSetting
		{
			UUID id;
			UUID type;
		};

		class EditorSimulatorInterface
		{
		public:
			virtual ~EditorSimulatorInterface() {}
			virtual void dispatch_message_to_editor(const char* data, size_t size) = 0;

			virtual SimulatorToolchainConfiguration get_toolchain_configuration() const = 0;
		};
	}
}
#endif