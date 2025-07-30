MESSAGE("Configuring Vadon Editor Demo")

# FIXME: a lot of this logic is the same as the engine targets
# Find some way to deduplicate?
set(VADONDEMO_CURRENT_TARGET "VadonDemoEditor")

add_executable(${VADONDEMO_CURRENT_TARGET})

add_subdirectory(VadonDemo)

get_property("VADONEDITORDEMO_SOURCES" TARGET ${VADONDEMO_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONEDITORDEMO_SOURCES})

target_include_directories(${VADONDEMO_CURRENT_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set(VADONEDITORDEMO_LIBRARIES VadonEditor VadonDemoCommon)
target_link_libraries(${VADONDEMO_CURRENT_TARGET} PRIVATE ${VADONEDITORDEMO_LIBRARIES})
	
# Enable console window for debug builds, disable in release
if(WIN32)
    target_link_options(${VADONDEMO_CURRENT_TARGET} PRIVATE "$<IF:$<CONFIG:Debug>,/SUBSYSTEM:CONSOLE,/SUBSYSTEM:WINDOWS>" "/ENTRY:mainCRTStartup")
else()
	# Enable console window for debug builds, disable in release
    target_link_options(${VADONDEMO_CURRENT_TARGET} PRIVATE "$<IF:$<NOT:$<CONFIG:Debug>>,-mwindows,>")
endif()

set_target_properties(${VADONDEMO_CURRENT_TARGET} PROPERTIES FOLDER ${VADON_PARENT_FOLDER}${PROJECT_NAME})