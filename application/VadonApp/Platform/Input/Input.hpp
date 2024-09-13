#ifndef VADONAPP_PLATFORM_INPUT_INPUT_HPP
#define VADONAPP_PLATFORM_INPUT_INPUT_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
namespace VadonApp::Platform
{
	struct InputActionInfo
	{
		std::string name;
		// TODO: anything else?
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(InputAction, InputActionHandle);
}
#endif