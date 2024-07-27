#ifndef VADON_SCENE_RESOURCE_RESOURCE_HPP
#define VADON_SCENE_RESOURCE_RESOURCE_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace Vadon::Scene
{
	VADON_DECLARE_TYPED_POOL_HANDLE(Resource, ResourceHandle);
	using ResourceID = Vadon::Utilities::UUID;

	struct ResourceBase
	{
		ResourceID id;
		Vadon::Utilities::TypeID type_id;
		std::string name;

		virtual ~ResourceBase() {}
	};
}
#endif