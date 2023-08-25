MESSAGE("Building Vadon Graphics API")

set(VADON_CURRENT_TARGET "VadonGraphicsAPI")
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

add_library(Vadon::GraphicsAPI ALIAS ${VADON_CURRENT_TARGET})

get_property("VADONGRAPHICSAPI_SOURCES" TARGET ${VADON_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONGRAPHICSAPI_SOURCES})

add_subdirectory(Vadon)
  
target_compile_features(${VADON_CURRENT_TARGET} PRIVATE cxx_std_20)
target_include_directories(${VADON_CURRENT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(VADON_GRAPHICS_API_LIBRARIES Vadon::Render)
target_link_libraries(${VADON_CURRENT_TARGET} PUBLIC ${VADON_GRAPHICS_API_LIBRARIES})

# Link to custom target
target_link_libraries(${VADON_CORE_TARGET} PRIVATE ${VADON_CURRENT_TARGET})

if(WIN32)
	target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADON_PLATFORM_WIN32)
	
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO!!!
	else()
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADON_LINK_DYNAMIC)
		target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADONGRAPHICSAPI_EXPORTS)
	endif()
endif()

if(MSVC)
  target_compile_options(${VADON_CURRENT_TARGET} PRIVATE /W4 /WX)
else()
  target_compile_options(${VADON_CURRENT_TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

set_target_properties(${VADON_CURRENT_TARGET} PROPERTIES FOLDER ${VADON_PARENT_FOLDER}${PROJECT_NAME})