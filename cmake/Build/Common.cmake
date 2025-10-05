macro(VadonCreateEngineTarget TARGET_NAME TARGET_ALIAS DLLEXPORT_MACRO_NAME)

set(VADON_CURRENT_TARGET ${TARGET_NAME})
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

# Add an alias for referral
add_library("Vadon::${TARGET_ALIAS}" ALIAS ${VADON_CURRENT_TARGET})

target_include_directories(${VADON_CURRENT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add common build options
target_link_libraries(${VADON_CURRENT_TARGET} PRIVATE $<BUILD_INTERFACE:VadonBuildOptions>)

get_property("${VADON_CURRENT_TARGET}_SOURCES" TARGET ${VADON_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES "${VADON_CURRENT_TARGET}_SOURCES")

add_subdirectory(Vadon)

if(WIN32)
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO!!!
	else()
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE ${DLLEXPORT_MACRO_NAME})
	endif()
endif()

set_target_properties(${VADON_CURRENT_TARGET} PROPERTIES FOLDER ${VADON_PARENT_FOLDER}${PROJECT_NAME})

endmacro()