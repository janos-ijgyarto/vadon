#ifndef VADONEDITOR_PLATFORM_PLATFORMINTERFACE_HPP
#define VADONEDITOR_PLATFORM_PLATFORMINTERFACE_HPP
#include <VadonEditor/Platform/Module.hpp>
namespace VadonEditor::Platform
{
	class PlatformInterface : public PlatformSystem<PlatformInterface>
	{
	public:
		~PlatformInterface();
	private:
		PlatformInterface(Core::Editor& editor);

		bool initialize();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Editor;
	};
}
#endif