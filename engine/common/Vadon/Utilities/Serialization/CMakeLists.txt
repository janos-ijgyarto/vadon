add_subdirectory(JSON)

target_sources(${VADON_CURRENT_TARGET}
    PRIVATE
	Serializer.hpp
	Serializer.cpp
   )