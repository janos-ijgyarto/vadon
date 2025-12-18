function(VadonCreateBuildOptions)
	set(BUILD_OPTIONS_TARGET "VadonBuildOptions")

	# Interface library for common build options (inspired by SDL)
	add_library(${BUILD_OPTIONS_TARGET} INTERFACE)

	if(NOT DEFINED VADON_COMPILE_DEFINITIONS)
		if(WIN32)
			target_compile_definitions(${BUILD_OPTIONS_TARGET} INTERFACE VADON_PLATFORM_WIN32)
		
			if(${VADON_LINK_STATIC} STREQUAL "ON")
				# TODO!!!
			else()
				# Provide compile def for DLL export/import
				target_compile_definitions(${BUILD_OPTIONS_TARGET} INTERFACE VADON_LINK_DYNAMIC)
			endif()
		elseif(UNIX AND NOT APPLE)
			target_compile_definitions(${BUILD_OPTIONS_TARGET} INTERFACE VADON_PLATFORM_LINUX)
		endif()
	else()
		target_compile_definitions(${BUILD_OPTIONS_TARGET} INTERFACE ${VADON_COMPILE_DEFINITIONS})
	endif()

	if(NOT DEFINED VADON_COMPILE_FEATURES)
		target_compile_features(${BUILD_OPTIONS_TARGET} INTERFACE cxx_std_20)
	else()
		target_compile_features(${BUILD_OPTIONS_TARGET} INTERFACE ${VADON_COMPILE_FEATURES})
	endif()

	if(NOT DEFINED VADON_COMPILE_OPTIONS)
		if(MSVC)
			# NOTE: need to disable warning related to dll-interface
			target_compile_options(${BUILD_OPTIONS_TARGET} INTERFACE /W4 /WX /wd4251)
		else()
			target_compile_options(${BUILD_OPTIONS_TARGET} INTERFACE -Wall -Wextra -Wpedantic -Werror)
		endif()
	else()
		target_compile_options(${BUILD_OPTIONS_TARGET} INTERFACE ${VADON_COMPILE_OPTIONS})
	endif()
endfunction()