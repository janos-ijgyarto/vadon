cmake_minimum_required(VERSION 3.21)

MESSAGE("Building Vadon Demo")

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
  message(FATAL_ERROR "In-tree build attempted, aborting build.")
endif()

project(VadonDemo
	VERSION 0.0.1
	DESCRIPTION "Demo application using Vadon (with CMake)"
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

set(VADONDEMO_CURRENT_TARGET "VadonDemo")

if(WIN32)
	MESSAGE("Building from Windows")
# TODO: Windows-specific instructions
elseif(UNIX AND NOT APPLE)
	MESSAGE("Building from Linux")
# TODO: Linux-specific instructions
endif()

add_executable(${VADONDEMO_CURRENT_TARGET})

add_subdirectory(VadonDemo)

get_property("VADONDEMO_SOURCES" TARGET ${VADONDEMO_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONDEMO_SOURCES})

target_compile_features(${VADONDEMO_CURRENT_TARGET} PRIVATE cxx_std_20)
target_include_directories(${VADONDEMO_CURRENT_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set(VADONDEMO_LIBRARIES VadonApp)
target_link_libraries(${VADONDEMO_CURRENT_TARGET} PRIVATE ${VADONDEMO_LIBRARIES})

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

if(DEFINED ${VADON_PARENT_FOLDER})
	set_target_properties(${VADONDEMO_CURRENT_TARGET} PROPERTIES FOLDER ${VADON_PARENT_FOLDER})
endif()