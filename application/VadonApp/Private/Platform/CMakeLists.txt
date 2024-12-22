add_subdirectory(Event)
add_subdirectory(Input)
add_subdirectory(Null)

# TODO: warning/error if invalid platform is selected!
if(VADON_PLATFORM_INTERFACE STREQUAL "DEFAULT" OR VADON_PLATFORM_INTERFACE STREQUAL "SDL")
	add_subdirectory(SDL)
endif()

target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
	PlatformInterface.hpp
	PlatformInterface.cpp
   )