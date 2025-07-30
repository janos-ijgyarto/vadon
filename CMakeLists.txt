cmake_minimum_required(VERSION 4.1)

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
  message(FATAL_ERROR "In-tree build attempted, aborting build.")
endif()

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

# Interface library for common build options (inspired by SDL)
add_library(VadonBuildOptions INTERFACE)

if(NOT DEFINED VADON_COMPILE_DEFINITIONS)
	if(WIN32)
		target_compile_definitions(VadonBuildOptions INTERFACE VADON_PLATFORM_WIN32)
		
		if(${VADON_LINK_STATIC} STREQUAL "ON")
			# TODO!!!
		else()
			# Provide compile def for DLL export/import
			target_compile_definitions(VadonBuildOptions INTERFACE VADON_LINK_DYNAMIC)
		endif()
	elseif(UNIX AND NOT APPLE)
		target_compile_definitions(VadonBuildOptions INTERFACE VADON_PLATFORM_LINUX)
	endif()
else()
	target_compile_definitions(VadonBuildOptions INTERFACE ${VADON_COMPILE_DEFINITIONS})
endif()

if(NOT DEFINED VADON_COMPILE_FEATURES)
	target_compile_features(VadonBuildOptions INTERFACE cxx_std_20)
else()
	target_compile_features(VadonBuildOptions INTERFACE ${VADON_COMPILE_FEATURES})
endif()

if(NOT DEFINED VADON_COMPILE_OPTIONS)
	if(MSVC)
		# NOTE: need to disable warning related to dll-interface
		target_compile_options(VadonBuildOptions INTERFACE /W4 /WX /wd4251)
	else()
		target_compile_options(VadonBuildOptions INTERFACE -Wall -Wextra -Wpedantic -Werror)
	endif()
else()
	target_compile_options(VadonBuildOptions INTERFACE ${VADON_COMPILE_OPTIONS})
endif()

if(${VADON_LINK_STATIC} STREQUAL "ON")
	MESSAGE("Configuring Vadon with static linkage")
else()
	MESSAGE("Configuring Vadon with dynamic linkage")
endif()

# Set the runtime output dir for all sub-projects
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# For tools, we want to put them next to the preset configs (the tools themselves are built in Release by default)
set(VADON_TOOLS_BINARY_DIR "${VADON_BUILD_DIR_ROOT}/tools")
set(VADON_TOOLS_EXPORT_FILE_DIR "${VADON_TOOLS_BINARY_DIR}/cmake")

set(VADON_INSTALL_RUNTIME_DESTINATION "bin")
set(VADON_INSTALL_THIRD_PARTY_COMPONENT "VadonThirdParty")

set(VADON_TOOLS_BUILD_OPTIONS "")

# Compiler settings for tools
if(DEFINED VADON_TOOLS_COMPILE_DEFINITIONS)
	list(APPEND VADON_TOOLS_BUILD_OPTIONS "-DVADON_TOOLS_COMPILE_DEFINITIONS=${VADON_TOOLS_COMPILE_DEFINITIONS}")
endif()

if(DEFINED VADON_TOOLS_COMPILE_FEATURES)
	list(APPEND VADON_TOOLS_BUILD_OPTIONS "-DVADON_TOOLS_COMPILE_FEATURES=${VADON_TOOLS_COMPILE_FEATURES}")
endif()

if(DEFINED VADON_TOOLS_COMPILE_OPTIONS)
	list(APPEND VADON_TOOLS_BUILD_OPTIONS "-DVADON_TOOLS_COMPILE_OPTIONS=${VADON_TOOLS_COMPILE_OPTIONS}")
endif()

# Run CMake to configure all of our tools
execute_process(COMMAND ${CMAKE_COMMAND}
	-S "${CMAKE_SOURCE_DIR}/tools"
	-B ${VADON_TOOLS_BINARY_DIR}
	-G ${CMAKE_GENERATOR}
	-DCMAKE_BUILD_TYPE=Release
	-DVADON_TOOLS_EXPORT=ON
	${VADON_TOOLS_BUILD_OPTIONS}
	)

add_custom_target(VadonToolsBuild
	COMMAND ${CMAKE_COMMAND} --build ${VADON_TOOLS_BINARY_DIR}
)

add_subdirectory(engine)

option(VADON_ENABLE_APPLICATION "Add VadonApp library" ON)
option(VADON_ENABLE_DEMO "Add Vadon Demo app" OFF)

if(${VADON_ENABLE_APPLICATION} STREQUAL "ON")
	add_subdirectory(application)
	add_subdirectory(editor)
	
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