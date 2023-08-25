# TODO: warning/error if invalid platform is selected!
if(VADON_PLATFORM_INTERFACE STREQUAL "DEFAULT" OR VADON_PLATFORM_INTERFACE STREQUAL "SDL")
	add_subdirectory(SDL2)
endif()

add_subdirectory(ImGui)