cmake_minimum_required(VERSION 4.1)

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
  message(FATAL_ERROR "In-tree build attempted, aborting build.")
endif()

message(STATUS "Configuring Vadon tools")

option(VADON_TOOLS_EXPORT "Export tools to parent project" OFF)

project(VadonTools
	VERSION 0.0.1
	DESCRIPTION "Vadon Engine Tools (with CMake)"
	LANGUAGES C CXX)

# Make sure all tools put their export file in a common directory
set(VADON_TOOLS_EXPORT_FILE_DIR "${CMAKE_BINARY_DIR}/cmake")

# Interface library for common build options (inspired by SDL)
add_library(VadonToolsBuildOptions INTERFACE)

if(NOT DEFINED VADON_TOOLS_COMPILE_DEFINITIONS)
	if(WIN32)
		target_compile_definitions(VadonToolsBuildOptions INTERFACE VADON_PLATFORM_WIN32)
	elseif(UNIX AND NOT APPLE)
		target_compile_definitions(VadonToolsBuildOptions INTERFACE VADON_PLATFORM_LINUX)
	endif()
else()
	target_compile_definitions(VadonToolsBuildOptions INTERFACE ${VADON_TOOLS_COMPILE_DEFINITIONS})
endif()

if(NOT DEFINED VADON_TOOLS_COMPILE_FEATURES)
	target_compile_features(VadonToolsBuildOptions INTERFACE cxx_std_20)
else()
	target_compile_features(VadonToolsBuildOptions INTERFACE ${VADON_TOOLS_COMPILE_FEATURES})
endif()

if(NOT DEFINED VADON_TOOLS_COMPILE_OPTIONS)
	if(MSVC)
		# NOTE: need to disable warning related to dll-interface
		target_compile_options(VadonToolsBuildOptions INTERFACE /W4 /WX)
	else()
		target_compile_options(VadonToolsBuildOptions INTERFACE -Wall -Wextra -Wpedantic -Werror)
	endif()
else()
	target_compile_options(VadonToolsBuildOptions INTERFACE ${VADON_TOOLS_COMPILE_OPTIONS})
endif()

# TODO: any other top-level settings for tools?
add_subdirectory(shader_compiler)