MESSAGE("Configuring Vadon Application")

project(VadonApp
        VERSION 0.0.1
        DESCRIPTION "Vadon Application Library (with CMake)"
        LANGUAGES C CXX)

# FIXME: a lot of this logic is the same as the engine targets
# Find some way to deduplicate?
set(VADONAPP_CURRENT_TARGET ${PROJECT_NAME})

if(WIN32)
	# TODO: Windows-specific instructions
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		add_library(${VADONAPP_CURRENT_TARGET} STATIC)
	else()
		add_library(${VADONAPP_CURRENT_TARGET} SHARED)
	endif()
elseif(UNIX AND NOT APPLE)
	# TODO: Linux-specific instructions
	# FIXME: shared lib needs to be compiled differently if it links to static libs
	add_library(${VADONAPP_CURRENT_TARGET} STATIC)
endif()

# Add common build options
target_link_libraries(${VADONAPP_CURRENT_TARGET} PRIVATE VadonBuildOptions)

# Set SDL as default platform interface
if(NOT DEFINED VADON_PLATFORM_INTERFACE)
	set(VADON_PLATFORM_INTERFACE "DEFAULT")
endif()

set(VADONAPP_SHADERS_TARGET "VadonAppShaders")
add_custom_target(${VADONAPP_SHADERS_TARGET})

include(${VADON_TOOLS_EXPORT_FILE_DIR}/ShaderCompiler.cmake)
add_dependencies(${VADONAPP_SHADERS_TARGET} VadonToolsBuild)
set_target_properties(${VADONAPP_SHADERS_TARGET} PROPERTIES FOLDER "${VADON_PARENT_FOLDER}${PROJECT_NAME}/Codegen")

add_dependencies(${VADONAPP_CURRENT_TARGET} ${VADONAPP_SHADERS_TARGET})

set(VADONAPP_SHADERS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(VADONAPP_CODEGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/codegen")
set(VADONAPP_SHADER_OUTPUT_DIR "${VADONAPP_CODEGEN_DIR}/shaders")

add_subdirectory(third_party)
add_subdirectory(VadonApp)

get_property("VADONAPP_SOURCES" TARGET ${VADONAPP_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONAPP_SOURCES})

target_include_directories(${VADONAPP_CURRENT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${VADONAPP_CURRENT_TARGET} PRIVATE ${VADONAPP_SHADER_OUTPUT_DIR})

set(VADONAPP_LIBRARIES Vadon::Vadon)
target_link_libraries(${VADONAPP_CURRENT_TARGET} PUBLIC ${VADONAPP_LIBRARIES})

if(WIN32)
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO!!!
	else()	
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADONAPP_CURRENT_TARGET} PRIVATE VADONAPP_EXPORTS)
	endif()
endif()

set_target_properties(${VADONAPP_CURRENT_TARGET} PROPERTIES FOLDER "${VADON_PARENT_FOLDER}${PROJECT_NAME}")

set_target_properties(${VADONAPP_CURRENT_TARGET}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)