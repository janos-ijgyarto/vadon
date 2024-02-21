add_subdirectory(Concurrent)
add_subdirectory(DataBuffer)
add_subdirectory(ObjectPool)

target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	Box.hpp
	MappedStaticArray.hpp
	UniqueVector.hpp
	)