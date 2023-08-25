cmake_minimum_required(VERSION 3.21)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT DEFINED VADON_CPM_EXTERNAL)
	# Use CPM for package management
	include(CPM)

	# Redirect CPM source cache to provided path
	set(ORIGINAL_CPM_SOURCE_CACHE ${CPM_SOURCE_CACHE})
	if(NOT DEFINED VADON_CPM_SOURCE_CACHE)
		get_filename_component(VADON_CPM_SOURCE_CACHE "../VadonCPMCache" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
	endif()

	set(CPM_SOURCE_CACHE ${VADON_CPM_SOURCE_CACHE})
endif()

set(BUILD_SHARED_LIBS ON)

project(VADON
	VERSION 0.0.1
	DESCRIPTION "Vadon Engine (with CMake)"
	LANGUAGES C CXX)
	
if(NOT DEFINED VADON_CONFIGURATION_NAME)
	set(VADON_CONFIGURATION_NAME "VadonDefaultConfig")
endif()

if(NOT DEFINED VADON_PARENT_FOLDER)
	set(VADON_PARENT_FOLDER "")
endif()

option(VADON_LINK_STATIC "Build Vadon with static linkage only" OFF)

if(${VADON_LINK_STATIC} STREQUAL "ON")
	MESSAGE("Building Vadon with static linkage")
else()
	MESSAGE("Building Vadon with dynamic linkage")
endif()

add_subdirectory(engine)

option(VADON_ENABLE_APPLICATION "Add VadonApp library" ON)
option(VADON_ENABLE_DEMO "Add Vadon Demo app" OFF)

if(${VADON_ENABLE_APPLICATION} STREQUAL "ON")
	add_subdirectory(application)
	
	if(${VADON_ENABLE_DEMO} STREQUAL "ON")
		add_subdirectory(demo)
	endif()
endif()

add_custom_target(Readme SOURCES README.md)

if(MSVC OR ${CMAKE_GENERATOR} MATCHES "Xcode")
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake Targets")
endif()

if(NOT DEFINED VADON_CPM_EXTERNAL)
	# Reset CPM source cache after we are done
	set(CPM_SOURCE_CACHE ${ORIGINAL_CPM_SOURCE_CACHE})
endif()