add_subdirectory(Null)

if(NOT DEFINED VADON_GRAPHICS)
	set(VADON_GRAPHICS "DEFAULT")
endif()

# TODO: warning/error if invalid graphics API is selected!
if(WIN32)
	if(VADON_GRAPHICS STREQUAL "DEFAULT" OR VADON_GRAPHICS STREQUAL "DIRECTX")
		add_subdirectory(DirectX)
	endif()
	# TODO: other options!
endif()

if(UNIX AND NOT APPLE)
	# TODO: set up OpenGL for Linux?
endif()

target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	GraphicsAPI.hpp
	GraphicsAPI.cpp
	)