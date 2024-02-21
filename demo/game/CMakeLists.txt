MESSAGE("Building Vadon Game Demo")

set(VADONDEMO_CURRENT_TARGET "VadonDemoGame")

if(WIN32)
	MESSAGE("Building from Windows")
# TODO: Windows-specific instructions
elseif(UNIX AND NOT APPLE)
	MESSAGE("Building from Linux")
# TODO: Linux-specific instructions
endif()

add_executable(${VADONDEMO_CURRENT_TARGET})

add_subdirectory(VadonDemo)

get_property("VADONGAMEDEMO_SOURCES" TARGET ${VADONDEMO_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONGAMEDEMO_SOURCES})

target_compile_features(${VADONDEMO_CURRENT_TARGET} PRIVATE cxx_std_20)
target_include_directories(${VADONDEMO_CURRENT_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set(VADONGAMEDEMO_LIBRARIES VadonApp VadonDemoCommon)
target_link_libraries(${VADONDEMO_CURRENT_TARGET} PRIVATE ${VADONGAMEDEMO_LIBRARIES})

if(MSVC)
  target_compile_options(${VADONDEMO_CURRENT_TARGET} PRIVATE /W4 /WX /wd4554)
endif()
	
if(WIN32)
	target_compile_definitions(${VADONDEMO_CURRENT_TARGET} PRIVATE VADONDEMO_PLATFORM_WIN32)
	
    # Enable console window for debug builds, disable in release
    set(SUBSYSTEM_LINKER_OPTIONS "$<IF:$<CONFIG:Debug>,/SUBSYSTEM:CONSOLE,/SUBSYSTEM:WINDOWS>" "/ENTRY:mainCRTStartup")
else()
	# Enable console window for debug builds, disable in release
    set(SUBSYSTEM_LINKER_OPTIONS "$<IF:$<NOT:$<CONFIG:Debug>>,-mwindows,>")
endif()

target_link_options(${VADONDEMO_CURRENT_TARGET} PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})

if(UNIX AND NOT APPLE)
	target_compile_definitions(${VADONDEMO_CURRENT_TARGET} PRIVATE VADONDEMO_PLATFORM_LINUX)
endif()

# TODO: Mac support?

set_target_properties(${VADONDEMO_CURRENT_TARGET} PROPERTIES FOLDER ${VADON_PARENT_FOLDER}${PROJECT_NAME})