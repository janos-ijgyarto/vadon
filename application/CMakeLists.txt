cmake_minimum_required(VERSION 4.1)

MESSAGE("Building Vadon Application")

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
  message(FATAL_ERROR "In-tree build attempted, aborting build.")
endif()

project(VadonApp
        VERSION 0.0.1
        DESCRIPTION "Vadon Application Library (with CMake)"
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

set(VADONAPP_CURRENT_TARGET ${PROJECT_NAME})

if(WIN32)
	MESSAGE("Building from Windows")
	# TODO: Windows-specific instructions
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		add_library(${VADONAPP_CURRENT_TARGET} STATIC)
	else()
		add_library(${VADONAPP_CURRENT_TARGET} SHARED)
	endif()
elseif(UNIX AND NOT APPLE)
	MESSAGE("Building from Linux")
	# TODO: Linux-specific instructions
	# FIXME: shared lib needs to be compiled differently if it links to static libs
	add_library(${VADONAPP_CURRENT_TARGET} STATIC)
endif()

include(${VADON_TOOLS_EXPORT_FILE_DIR}/VadonShaderCompiler.cmake)

# Set SDL as default platform interface
if(NOT DEFINED VADON_PLATFORM_INTERFACE)
	set(VADON_PLATFORM_INTERFACE "DEFAULT")
endif()

set(VADONAPP_SHADERS_TARGET "VadonAppShaders")
add_custom_target(${VADONAPP_SHADERS_TARGET})

add_dependencies(${VADONAPP_SHADERS_TARGET} "VadonToolsBuild")
set_target_properties(${VADONAPP_SHADERS_TARGET} PROPERTIES FOLDER "${VADON_PARENT_FOLDER}${PROJECT_NAME}/Codegen")

add_dependencies(${VADONAPP_CURRENT_TARGET} ${VADONAPP_SHADERS_TARGET})

set(VADONAPP_SHADERS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(VADONAPP_CODEGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/codegen")
set(VADONAPP_SHADER_OUTPUT_DIR "${VADONAPP_CODEGEN_DIR}/shaders")

add_subdirectory(third_party)
add_subdirectory(VadonApp)

get_property("VADONAPP_SOURCES" TARGET ${VADONAPP_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONAPP_SOURCES})
  
target_compile_features(${VADONAPP_CURRENT_TARGET} PRIVATE cxx_std_20)
target_include_directories(${VADONAPP_CURRENT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${VADONAPP_CURRENT_TARGET} PRIVATE ${VADONAPP_SHADER_OUTPUT_DIR})

set(VADONAPP_LIBRARIES Vadon::Vadon)
target_link_libraries(${VADONAPP_CURRENT_TARGET} PUBLIC ${VADONAPP_LIBRARIES})

if(WIN32)
	target_compile_definitions(${VADONAPP_CURRENT_TARGET} PRIVATE VADONAPP_PLATFORM_WIN32)
	
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO!!!
	else()	
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADONAPP_CURRENT_TARGET} PRIVATE VADON_LINK_DYNAMIC)
		target_compile_definitions(${VADONAPP_CURRENT_TARGET} PRIVATE VADONAPP_EXPORTS)
	endif()
endif()

if(UNIX AND NOT APPLE)
	target_compile_definitions(${VADONAPP_CURRENT_TARGET} PRIVATE PLATFORM_LINUX)
endif()

# TODO: Mac support?

set_target_properties(${VADONAPP_CURRENT_TARGET} PROPERTIES FOLDER "${VADON_PARENT_FOLDER}${PROJECT_NAME}")