add_subdirectory(Encoding)
add_subdirectory(Hash)

target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	DataUtilities.hpp
	Variant.hpp
	VariantArray.hpp
	VariantBase.hpp
	VariantDictionary.hpp
	Visitor.hpp
	)