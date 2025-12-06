MESSAGE("Configuring Vadon Demo Common")

# FIXME: a lot of this logic is the same as the engine targets
# Find some way to deduplicate?
set(VADONDEMO_CURRENT_TARGET "VadonDemoCommon")

if(WIN32)
	# TODO: Windows-specific instructions
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		add_library(${VADONDEMO_CURRENT_TARGET} STATIC)
	else()
		add_library(${VADONDEMO_CURRENT_TARGET} SHARED)
	endif()
elseif(UNIX AND NOT APPLE)
	# TODO: Linux-specific instructions
	# FIXME: shared lib needs to be compiled differently if it links to static libs
	add_library(${VADONDEMO_CURRENT_TARGET} STATIC)
endif()

# Add common build options
target_link_libraries(${VADONDEMO_CURRENT_TARGET} PUBLIC VadonBuildOptions)

get_property("VADONDEMOCOMMON_SOURCES" TARGET ${VADONDEMO_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONCOMMON_SOURCES})

add_subdirectory(VadonDemo)
  
target_include_directories(${VADONDEMO_CURRENT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(VADONDEMOCOMMON_LIBRARIES Vadon::Common Vadon::Render)
target_link_libraries(${VADONDEMO_CURRENT_TARGET} PUBLIC ${VADONDEMOCOMMON_LIBRARIES})

if(WIN32)
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO
	else()
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADONDEMO_CURRENT_TARGET} PRIVATE VADONDEMO_EXPORTS)
	endif()
endif()

set_target_properties(${VADONDEMO_CURRENT_TARGET} PROPERTIES FOLDER ${VADON_PARENT_FOLDER}${PROJECT_NAME})

set_target_properties(${VADONDEMO_CURRENT_TARGET}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)