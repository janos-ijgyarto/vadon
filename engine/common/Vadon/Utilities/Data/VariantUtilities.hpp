#ifndef VADON_UTILITIES_DATA_VARIANTUTILITIES_HPP
#define VADON_UTILITIES_DATA_VARIANTUTILITIES_HPP
namespace Vadon::Utilities
{
	// Utility for visiting std::variant with Overload Pattern
	template<typename... Ts>
	struct VisitorOverloadList : Ts... { using Ts::operator()...; };
}
#endif