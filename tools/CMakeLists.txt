message(STATUS "Configuring Vadon tools")

option(VADON_TOOLS_EXPORT "Export tools to parent project" OFF)
option(VADON_TOOLS_ENABLE_EDITOR "Include Editor project in tools build" OFF)

project(VadonTools
	VERSION 0.0.1
	DESCRIPTION "Vadon Engine Dev Tools (with CMake)"
	LANGUAGES C CXX)

# Make sure all tools put their export file in a common directory
# TODO: allow setting this via configuration parameters!
set(VADON_TOOLS_EXPORT_FILE_DIR "${CMAKE_BINARY_DIR}/cmake")

add_subdirectory(third_party)
if(VADON_TOOLS_ENABLE_EDITOR)
	add_subdirectory(editor)
endif()
add_subdirectory(shader_compiler)