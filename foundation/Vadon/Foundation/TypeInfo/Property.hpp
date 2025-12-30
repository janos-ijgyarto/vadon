#ifndef VADON_FOUNDATION_TYPEINFO_PROPERTY_HPP
#define VADON_FOUNDATION_TYPEINFO_PROPERTY_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
namespace Vadon
{
	namespace Foundation
	{
		struct Property
		{
			UUID id;
			UUID type;
			// TODO: any other metadata?
		};
	}
}
#endif