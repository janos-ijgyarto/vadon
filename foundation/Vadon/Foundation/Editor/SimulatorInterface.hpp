#ifndef VADON_FOUNDATION_EDITOR_SIMULATORINTERFACE_HPP
#define VADON_FOUNDATION_EDITOR_SIMULATORINTERFACE_HPP
namespace Vadon
{
	namespace Foundation
	{
		class EditorSimulatorInterface
		{
		public:
			virtual ~EditorSimulatorInterface() {}
			virtual void dispatch_message_to_editor(const char* data, size_t size) = 0;
		};
	}
}
#endif