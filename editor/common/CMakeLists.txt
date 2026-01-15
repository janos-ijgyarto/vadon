# FIXME: a lot of this logic is the same as the engine targets
# Find some way to deduplicate?
set(VADONEDITOR_CURRENT_TARGET "VadonEditorPluginCommon")

if(WIN32)
	# TODO: Windows-specific instructions
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		add_library(${VADONEDITOR_CURRENT_TARGET} STATIC)
	else()
		add_library(${VADONEDITOR_CURRENT_TARGET} SHARED)
	endif()
elseif(UNIX AND NOT APPLE)
	MESSAGE("Building from Linux")
	# TODO: Linux-specific instructions
	# FIXME: shared lib needs to be compiled differently if it links to static libs
	add_library(${VADONEDITOR_CURRENT_TARGET} STATIC)
endif()

# Add common build options
target_link_libraries(${VADONEDITOR_CURRENT_TARGET} PRIVATE VadonBuildOptions Vadon::Common)

add_subdirectory(VadonEditor)

get_property("VADONEDITOR_SOURCES" TARGET ${VADONEDITOR_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONEDITOR_SOURCES})

target_include_directories(${VADONEDITOR_CURRENT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${VADONEDITOR_CURRENT_TARGET} PUBLIC VadonFoundation)

if(WIN32)
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO!!!
	else()	
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADONEDITOR_CURRENT_TARGET} PRIVATE "VADONEDITOR_EXPORTS")
	endif()
endif()

set_target_properties(${VADONEDITOR_CURRENT_TARGET} PROPERTIES FOLDER "${VADON_PARENT_FOLDER}${PROJECT_NAME}")

set_target_properties(${VADONEDITOR_CURRENT_TARGET}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)