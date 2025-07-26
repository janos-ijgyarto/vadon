cmake_minimum_required(VERSION 4.1)

MESSAGE("Building Vadon Editor")

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
  message(FATAL_ERROR "In-tree build attempted, aborting build.")
endif()

project(VadonEditor
	VERSION 0.0.1
	DESCRIPTION "Vadon Engine Editor (with CMake)"
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

set(VADONEDITOR_CURRENT_TARGET ${PROJECT_NAME})

if(WIN32)
	MESSAGE("Building from Windows")
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

add_subdirectory(VadonEditor)

get_property("VADONEDITOR_SOURCES" TARGET ${VADONEDITOR_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONEDITOR_SOURCES})

target_compile_features(${VADONEDITOR_CURRENT_TARGET} PRIVATE cxx_std_20)
target_include_directories(${VADONEDITOR_CURRENT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(VADONEDITOR_LIBRARIES VadonApp)
target_link_libraries(${VADONEDITOR_CURRENT_TARGET} PUBLIC ${VADONEDITOR_LIBRARIES})

if(WIN32)
	target_compile_definitions(${VADONEDITOR_CURRENT_TARGET} PRIVATE VADONEDITOR_PLATFORM_WIN32)
	
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO!!!
	else()	
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADONEDITOR_CURRENT_TARGET} PRIVATE VADON_LINK_DYNAMIC)
		target_compile_definitions(${VADONEDITOR_CURRENT_TARGET} PRIVATE VADONEDITOR_EXPORTS)
	endif()
endif()
	
if(UNIX AND NOT APPLE)
	target_compile_definitions(${VADONEDITOR_CURRENT_TARGET} PRIVATE VADONEDITOR_PLATFORM_LINUX)
endif()

# TODO: Mac support?

if(DEFINED ${VADON_PARENT_FOLDER})
	set_target_properties(${VADONEDITOR_CURRENT_TARGET} PROPERTIES FOLDER ${VADON_PARENT_FOLDER})
endif()