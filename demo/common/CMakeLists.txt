MESSAGE("Building Vadon Demo Common")

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

get_property("VADONDEMOCOMMON_SOURCES" TARGET ${VADONDEMO_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONCOMMON_SOURCES})

add_subdirectory(VadonDemo)
  
target_compile_features(${VADONDEMO_CURRENT_TARGET} PRIVATE cxx_std_20)
target_include_directories(${VADONDEMO_CURRENT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(VADONDEMOCOMMON_LIBRARIES Vadon::Render)
target_link_libraries(${VADONDEMO_CURRENT_TARGET} PUBLIC ${VADONDEMOCOMMON_LIBRARIES})

if(WIN32)
	target_compile_definitions(${VADONDEMO_CURRENT_TARGET} PRIVATE VADONDEMO_PLATFORM_WIN32)
	
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO
	else()
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADONDEMO_CURRENT_TARGET} PRIVATE VADON_LINK_DYNAMIC)
		target_compile_definitions(${VADONDEMO_CURRENT_TARGET} PRIVATE VADONDEMO_EXPORTS)
	endif()
endif()

if(MSVC)
  target_compile_options(${VADONDEMO_CURRENT_TARGET} PRIVATE /W4 /WX /wd4251)
  target_compile_options(${VADONDEMO_CURRENT_TARGET} PUBLIC)
else()
  target_compile_options(${VADONDEMO_CURRENT_TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

set_target_properties(${VADONDEMO_CURRENT_TARGET} PROPERTIES FOLDER ${VADON_PARENT_FOLDER}${PROJECT_NAME})