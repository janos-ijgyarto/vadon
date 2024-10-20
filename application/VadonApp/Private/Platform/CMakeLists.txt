add_subdirectory(Dummy)
add_subdirectory(Event)
add_subdirectory(Input)

# TODO: warning/error if invalid platform is selected!
if(VADON_PLATFORM_INTERFACE STREQUAL "DEFAULT" OR VADON_PLATFORM_INTERFACE STREQUAL "SDL")
	add_subdirectory(SDL)
endif()

target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
	PlatformInterface.hpp
	PlatformInterface.cpp
   )