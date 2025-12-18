cmake_minimum_required(VERSION 4.1)

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
  message(FATAL_ERROR "In-tree build attempted, aborting build.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Build/Common)
include(Build/Engine)

if(NOT DEFINED VADON_CPM_EXTERNAL)
	# Use CPM for package management
	include(CPM)

	# Redirect CPM source cache to provided path
	set(ORIGINAL_CPM_SOURCE_CACHE ${CPM_SOURCE_CACHE})
	if(NOT DEFINED VADON_CPM_SOURCE_CACHE)
		set(VADON_CPM_SOURCE_CACHE "${VADON_BUILD_DIR_ROOT}/VadonCPMCache")
	endif()

	set(CPM_SOURCE_CACHE ${VADON_CPM_SOURCE_CACHE})
endif()

set(BUILD_SHARED_LIBS ON)

project(VADON
	VERSION 0.0.1
	DESCRIPTION "Vadon Engine (with CMake)"
	LANGUAGES C CXX)

if(NOT DEFINED VADON_PLATFORM)
	# TODO: get platform from CMake
endif()

if(NOT DEFINED VADON_CONFIGURATION)
	# TODO: get configuration (debug/release/etc.) from CMake
endif()

if(NOT DEFINED VADON_COMPILER)
	# TODO: get compiler from CMake
endif()

# TODO: compiler, etc.

if(NOT DEFINED VADON_PARENT_FOLDER)
	set(VADON_PARENT_FOLDER "")
endif()

option(VADON_LINK_STATIC "Build Vadon with static linkage only" OFF)
option(VADON_ENABLE_APPLICATION "Add VadonApp library" ON)
option(VADON_ENABLE_DEMO "Add Vadon Demo app" OFF)

VadonCreateBuildOptions()

if(${VADON_LINK_STATIC} STREQUAL "ON")
	MESSAGE("Configuring Vadon with static linkage")
else()
	MESSAGE("Configuring Vadon with dynamic linkage")
endif()

# Add README
add_custom_target(Readme SOURCES README.md)

# Set some IDE-specific features
if(MSVC OR ${CMAKE_GENERATOR} MATCHES "Xcode")
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake Targets")
endif()

# FIXME: we allow either only engine or only tools configuration
# Doing both at the same time is more complicated (need to make sure engine has access to all necessary tools)
if(${VADON_CONFIGURATION_MODE} STREQUAL "ENGINE")
	option(VADON_ENABLE_APPLICATION "Add VadonApp library" ON)
	option(VADON_ENABLE_DEMO "Add Vadon Demo app" OFF)

	set(VADON_INSTALL_RUNTIME_DESTINATION "bin")
	set(VADON_INSTALL_THIRD_PARTY_COMPONENT "VadonThirdParty")

	# NOTE: tools are handled as a "separate project" because we only need them in a specific configuration
	# We only need multiple configurations if we want to debug them
	set(VADON_ENGINE_TOOLS_PRESET_NAME "tools-${VADON_PLATFORM}-${VADON_COMPILER}-release")

	# Provide variable for targets to find the exported tools
	# FIXME: for the engine build tools, set a specific path, or find a way to get it from the other build tree!
	set(VADON_TOOLS_EXPORT_FILE_DIR "${VADON_BUILD_DIR_ROOT}/tools/${VADON_ENGINE_TOOLS_PRESET_NAME}/cmake")

	# Run CMake on the same root script, but this time using a tool preset
	execute_process(COMMAND ${CMAKE_COMMAND}
		-S ${CMAKE_CURRENT_SOURCE_DIR}
		--preset ${VADON_ENGINE_TOOLS_PRESET_NAME}
		-G ${CMAKE_GENERATOR}
		-DVADON_TOOLS_ENGINE_BUILD=ON
		-DVADON_TOOLS_EXPORT=ON
		RESULT_VARIABLE VADON_TOOLS_CONFIGURE_RESULT
		)

	if(NOT VADON_TOOLS_CONFIGURE_RESULT EQUAL "0")
		message( FATAL_ERROR "Failed to configure Vadon Tools!")
	endif()

	# FIXME: instead of building all tools, we could select specific targets?
	# FIXME2: find a way to build via the preset?
	add_custom_target(VadonToolsBuild
		COMMAND ${CMAKE_COMMAND} --build "${VADON_BUILD_DIR_ROOT}/tools/${VADON_ENGINE_TOOLS_PRESET_NAME}"
	)

	add_subdirectory(engine)

	if(${VADON_ENABLE_APPLICATION} STREQUAL "ON")
		add_subdirectory(application)
		add_subdirectory(editor)
	
		if(${VADON_ENABLE_DEMO} STREQUAL "ON")
			add_subdirectory(demo)
		endif()
	endif()
elseif(${VADON_CONFIGURATION_MODE} STREQUAL "TOOLS")
	add_subdirectory(tools)
endif()

if(NOT DEFINED VADON_CPM_EXTERNAL)
	# Reset CPM source cache after we are done
	set(CPM_SOURCE_CACHE ${ORIGINAL_CPM_SOURCE_CACHE})
endif()