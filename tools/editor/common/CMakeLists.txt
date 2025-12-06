set(VADONEDITOR_COMMON "VadonEditorCommon")

if(WIN32)
	# TODO: Windows-specific instructions
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		add_library(${VADONEDITOR_COMMON} STATIC)
	else()
		add_library(${VADONEDITOR_COMMON} SHARED)
	endif()
elseif(UNIX AND NOT APPLE)
	# TODO: Linux-specific instructions
	# FIXME: shared lib needs to be compiled differently if it links to static libs
	add_library(${VADONEDITOR_COMMON} STATIC)
endif()

target_include_directories(${VADONEDITOR_COMMON} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${VADONEDITOR_COMMON} PUBLIC VadonBuildOptions asio)

set_target_properties(${VADONEDITOR_COMMON} PROPERTIES FOLDER ${PROJECT_NAME})

set_target_properties(${VADONEDITOR_COMMON}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${VADON_EDITOR_RUNTIME_OUTPUT_DIRECTORY}
)

add_subdirectory(VadonEditor)

if(WIN32)
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO!!!
	else()
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADONEDITOR_COMMON} PRIVATE "VADONEDITORCOMMON_EXPORTS")
	endif()
endif()