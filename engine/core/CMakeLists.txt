MESSAGE("Building Vadon Core")

set(VADON_CURRENT_TARGET ${VADON_CORE_TARGET})
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
add_library(Vadon::Vadon ALIAS ${VADON_CURRENT_TARGET})

set_target_properties(${VADON_CURRENT_TARGET} PROPERTIES FOLDER ${VADON_PARENT_FOLDER}${PROJECT_NAME})

get_property("VADONCORE_SOURCES" TARGET ${VADON_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONCORE_SOURCES})

add_subdirectory(Vadon)
 
# Must compile with C++20
target_compile_features(${VADON_CURRENT_TARGET} PUBLIC cxx_std_20)

# Add include directories
target_include_directories(${VADON_CURRENT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
  target_compile_options(${VADON_CURRENT_TARGET} PRIVATE /W4 /WX)
else()
  target_compile_options(${VADON_CURRENT_TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(WIN32)
	target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADON_PLATFORM_WIN32)
	
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO!!!
	else()
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADON_LINK_DYNAMIC)
		target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADONCORE_EXPORTS)
	endif()
endif()

if(UNIX AND NOT APPLE)
	target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADON_PLATFORM_LINUX)
endif()

# TODO: Mac support?