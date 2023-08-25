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

# Add an alias for referral
add_library(Vadon::Render ALIAS ${VADON_CURRENT_TARGET})

get_property("VADONRENDER_SOURCES" TARGET ${VADON_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONRENDER_SOURCES})

add_subdirectory(Vadon)
  
target_compile_features(${VADON_CURRENT_TARGET} PRIVATE cxx_std_20)
target_include_directories(${VADON_CURRENT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(VADON_RENDER_LIBRARIES Vadon::Common)
target_link_libraries(${VADON_CURRENT_TARGET} PUBLIC ${VADON_RENDER_LIBRARIES})

# Add link to the main DLL
target_link_libraries(${VADON_CORE_TARGET} PUBLIC ${VADON_CURRENT_TARGET})

if(WIN32)
	target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADON_PLATFORM_WIN32)
	
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO!!!
	else()
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADON_LINK_DYNAMIC)
		target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADONRENDER_EXPORTS)
	endif()
endif()

if(MSVC)
  target_compile_options(${VADON_CURRENT_TARGET} INTERFACE /W4 /WX /wd4251)
else()
  target_compile_options(${VADON_CURRENT_TARGET} INTERFACE -Wall -Wextra -Wpedantic -Werror)
endif()

set_target_properties(${VADON_CURRENT_TARGET} PROPERTIES FOLDER ${VADON_PARENT_FOLDER}${PROJECT_NAME})