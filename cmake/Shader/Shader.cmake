function(vadon_generate_embedded_shader_source SHADER_TARGET SHADER_FILE SHADER_SOURCE_NAMESPACE)
	file(READ ${SHADER_FILE} SHADER_FILE_CONTENT)
	
	# FIXME: extract this to its own function?
	file(RELATIVE_PATH SHADER_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/render ${CMAKE_CURRENT_SOURCE_DIR})

	get_filename_component(SHADER_FILE_NAME ${SHADER_FILE} NAME_WE)
	# FIXME: path to custom CMake scripts root should be in a variable!
	configure_file(${CMAKE_SOURCE_DIR}/cmake/Shader/Shader.cpp.in ${SHADER_FILE_NAME}.cpp @ONLY)
	target_sources(${SHADER_TARGET}
	PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/${SHADER_FILE_NAME}.cpp
	)
endfunction()