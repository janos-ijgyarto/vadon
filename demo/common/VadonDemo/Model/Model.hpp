#ifndef VADONDEMO_MODEL_MODEL_HPP
#define VADONDEMO_MODEL_MODEL_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
namespace Vadon::Core
{
	class EngineCoreInterface;
}
namespace VadonDemo::Model
{
	class TestSystem;

	class Model
	{
	public:
		VADONDEMO_API Model(Vadon::Core::EngineCoreInterface& engine_core);

		VADONDEMO_API bool initialize();
		void update();	
	private:
		Vadon::Core::EngineCoreInterface& m_engine_core;
	};
}
#endif