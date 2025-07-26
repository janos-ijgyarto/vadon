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

# TODO: any other top-level settings for tools?
add_subdirectory(shader_compiler)