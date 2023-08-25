MESSAGE("Building Vadon Common")

set(VADON_CURRENT_TARGET "VadonCommon")
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
add_library(Vadon::Common ALIAS ${VADON_CURRENT_TARGET})

get_property("VADONCOMMON_SOURCES" TARGET ${VADON_CURRENT_TARGET} PROPERTY SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VADONCOMMON_SOURCES})

add_subdirectory(Vadon)
  
target_compile_features(${VADON_CURRENT_TARGET} PRIVATE cxx_std_20)
target_include_directories(${VADON_CURRENT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(VADON_PUBLIC_THIRD_PARTY concurrentqueue glm::glm nlohmann_json)
target_link_libraries(${VADON_CURRENT_TARGET} PUBLIC ${VADON_PUBLIC_THIRD_PARTY})

#set(PRIVATE_THIRD_PARTY )
#target_link_libraries(${VADON_CURRENT_TARGET} PUBLIC ${PRIVATE_THIRD_PARTY})

# Link to custom target
target_link_libraries(${VADON_CORE_TARGET} PUBLIC ${VADON_CURRENT_TARGET})

if(WIN32)
	target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADON_PLATFORM_WIN32)
	
	if(${VADON_LINK_STATIC} STREQUAL "ON")
		# TODO
	else()
		# Provide compile def for DLL export/import
		target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADON_LINK_DYNAMIC)
		target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADONCOMMON_EXPORTS)
	endif()
endif()

if(MSVC)
  target_compile_options(${VADON_CURRENT_TARGET} PRIVATE /W4 /WX /wd4251)
  target_compile_options(${VADON_CURRENT_TARGET} PUBLIC)
else()
  target_compile_options(${VADON_CURRENT_TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

set_target_properties(${VADON_CURRENT_TARGET} PROPERTIES FOLDER ${VADON_PARENT_FOLDER}${PROJECT_NAME})