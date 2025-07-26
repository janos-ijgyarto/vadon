# If we added this folder, we set the preprocessor definition to use SDL
target_compile_definitions(${VADONAPP_CURRENT_TARGET} PRIVATE VADON_PLATFORM_INTERFACE_SDL)

# Set the folder so our targets are organized
set(PREV_CMAKE_FOLDER ${CMAKE_FOLDER})
set(CMAKE_FOLDER "${VADON_PARENT_FOLDER}Dependencies/SDL")

# TODO: any config changes to make sure we only build what we need
set(SDL_TEST OFF)
set(SDL_TEST_ENABLED_BY_DEFAULT OFF)
set(SDL_LIBS SDL2main SDL2)

set(SDL2_DISABLE_INSTALL ON)

# Have to install manually so we only get what we need
CPMAddPackage(
  NAME sdl2
  URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.28.2.zip
  VERSION 2.28.2
)

if(MSVC)
	# Need to unset RTC, causes linker errors in SDL
	set_property(TARGET ${SDL_LIBS} PROPERTY MSVC_RUNTIME_CHECKS "")
endif()

target_link_libraries(${VADONAPP_CURRENT_TARGET} PRIVATE ${SDL_LIBS})

# Install the DLL only
install(TARGETS SDL2 RUNTIME COMPONENT ${VADON_INSTALL_THIRD_PARTY_COMPONENT} DESTINATION ${VADON_INSTALL_RUNTIME_DESTINATION})

# Reset folder
set(CMAKE_FOLDER ${PREV_CMAKE_FOLDER})