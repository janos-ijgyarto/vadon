cmake_minimum_required(VERSION 3.21)

MESSAGE("Building Vadon")

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
  message(FATAL_ERROR "In-tree build attempted, aborting build.")
endif()

project(VadonMain
        VERSION 0.0.1
        DESCRIPTION "Vadon implementation (with CMake)"
        LANGUAGES C CXX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	MESSAGE("Selected compiler: Clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	MESSAGE("Selected compiler: MSVC")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
	MESSAGE("Selected compiler: Intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	MESSAGE("Selected compiler: GNU (why???)")
endif()

set(VADON_CORE_TARGET "VadonCore")

add_subdirectory(core)
add_subdirectory(common)
add_subdirectory(render)
add_subdirectory(graphics_api)
add_subdirectory(third_party)