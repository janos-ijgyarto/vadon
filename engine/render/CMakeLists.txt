MESSAGE("Building Vadon Render")

set(VADON_CURRENT_TARGET "VadonRender")
if(WIN32)
	# TODO: Windows-specific instructions
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		add_library(${VADON_CURRENT_TARGET} STATIC)
	else()
		add_library(${VADON_CURRENT_TARGET} SHARED)
	endif()
elseif(UNIX AND NOT APPLE)
	# TODO: Linux-specific instructions
	# FIXME: shared lib needs to be compiled differently if it links to static libs
	add_library(${VADON_CURRENT_TARGET} STATIC)
endif()

include(${VADON_TOOLS_EXPORT_FILE_DIR}/VadonShaderCompiler.cmake)

set(VADONRENDER_SHADERS_TARGET "VadonRenderShaders")
add_custom_target(${VADONRENDER_SHADERS_TARGET})

add_dependencies(${VADONRENDER_SHADERS_TARGET} "VadonToolsBuild")
set_target_properties(${VADONRENDER_SHADERS_TARGET} PROPERTIES FOLDER "${VADON_PARENT_FOLDER}${PROJECT_NAME}/Codegen")

set(VADONRENDER_SHADERS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(VADONRENDER_CODEGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/codegen")
set(VADONRENDER_SHADER_OUTPUT_DIR "${VADONRENDER_CODEGEN_DIR}/shaders")

# Add an alias for referral
add_library(Vadon::Render ALIAS ${VADON_CURRENT_TARGET})
add_dependencies(${VADON_CURRENT_TARGET} ${VADONRENDER_SHADERS_TARGET})

get_property("VADONRENDER_SOURCES" TARGET ${VADON_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONRENDER_SOURCES})

add_subdirectory(Vadon)
  
target_compile_features(${VADON_CURRENT_TARGET} PRIVATE cxx_std_20)
target_include_directories(${VADON_CURRENT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${VADON_CURRENT_TARGET} PRIVATE ${VADONRENDER_SHADER_OUTPUT_DIR})

set(VADON_RENDER_LIBRARIES Vadon::Common freetype)
target_link_libraries(${VADON_CURRENT_TARGET} PUBLIC ${VADON_RENDER_LIBRARIES})

# Add link to the main DLL
target_link_libraries(${VADON_CORE_TARGET} PUBLIC ${VADON_CURRENT_TARGET})

if(WIN32)
	target_compile_definitions(${VADON_CURRENT_TARGET} PUBLIC VADON_PLATFORM_WIN32)
	
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO!!!
	else()
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADON_CURRENT_TARGET} PUBLIC VADON_LINK_DYNAMIC)
		target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADONRENDER_EXPORTS)
	endif()
endif()

if(MSVC)
  target_compile_options(${VADON_CURRENT_TARGET} INTERFACE /W4 /WX /wd4251)
else()
  target_compile_options(${VADON_CURRENT_TARGET} INTERFACE -Wall -Wextra -Wpedantic -Werror)
endif()

set_target_properties(${VADON_CURRENT_TARGET} PROPERTIES FOLDER ${VADON_PARENT_FOLDER}${PROJECT_NAME})